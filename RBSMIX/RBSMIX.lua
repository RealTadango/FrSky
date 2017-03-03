
--
-- Copyright (C) OpenTX
--
-- Based on code named
--   th9x - http://code.google.com/p/th9x
--   er9x - http://code.google.com/p/er9x
--   gruvin9x - http://code.google.com/p/gruvin9x
--
-- License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
--
-- This program is free software; you can redistribute it and/or modify
-- it under the terms of the GNU General Public License version 2 as
-- published by the Free Software Foundation.
--
-- This program is distributed in the hope that it will be useful,
-- but WITHOUT ANY WARRANTY; without even the implied warranty of
-- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
-- GNU General Public License for more details.
--

-- LUA Mix Script

local outputs = { "RFS" }
local prevStatus

local prevRx1FrameLoss = false
local prevRx2FrameLoss = false
local prevRx1Failsafe = false
local prevRx2Failsafe = false

local rx1FrameLossCount = 0
local rx2FrameLossCount = 0
local rx1FailsafeCount = 0
local rx2FailsafeCount = 0

local function bit(p)
  return 2 ^ (p - 1)  -- 1-based indexing
end

-- Typical call:  if hasbit(x, bit(3)) then ...
local function hasbit(x, p)
  return ((x % (p + p) >= p) and 1 or 0)
end

--  "Rx1 Ovl", "Rx2 Ovl", "SBUS Ovl", "Rx1 FS", "Rx1 LF", "Rx2 FS", "Rx2 LF", "Rx1 Lost", "Rx2 Lost", "Rx1 NS", "Rx2 NS"

local function run()
  local status = getValue("RBS")
  print("RBS VALUE "..status)
  setTelemetryValue(0xb20, 0, 2, hasbit(status, bit(4)), 0, 0, "R1FS")
  setTelemetryValue(0xb20, 1, 2, hasbit(status, bit(6)), 0, 0, "R2FS")
  setTelemetryValue(0xb20, 2, 2, hasbit(status, bit(5)), 0, 0, "R1FL")
  setTelemetryValue(0xb20, 3, 2, hasbit(status, bit(7)), 0, 0, "R2FL")

  if hasbit(status, bit(4)) == 1 then
	if prevRx1Failsafe == false then
		rx1FailsafeCount = rx1FailsafeCount + 1
	end
	prevRx1Failsafe = true
  else 
	prevRx1Failsafe = false
  end

  if hasbit(status, bit(6)) == 1 then
	if prevRx2Failsafe == false then
		rx2FailsafeCount = rx2FailsafeCount + 1
	end
	prevRx2Failsafe = true
  else 
	prevRx2Failsafe = false
  end

  if hasbit(status, bit(5)) == 1 then
	if prevRx1FrameLoss == false then
		rx1FrameLossCount = rx1FrameLossCount + 1
	end
	prevRx1FrameLoss = true
  else 
	prevRx1FrameLoss = false
  end

  if hasbit(status, bit(7)) == 1 then
	if prevRx2FrameLoss == false then
		rx2FrameLossCount = rx2FrameLossCount + 1
	end
	prevRx2FrameLoss = true
  else 
	prevRx2FrameLoss = false
  end
  
  setTelemetryValue(0xb20, 4, 2, rx1FrameLossCount, 0, 0, "R1LC")
  setTelemetryValue(0xb20, 5, 2, rx2FrameLossCount, 0, 0, "R2LC")
  setTelemetryValue(0xb20, 6, 2, rx1FailsafeCount, 0, 0, "R1HC")
  setTelemetryValue(0xb20, 7, 2, rx2FailsafeCount, 0, 0, "R2HC")

  
  if status ~= 0 then
    return 10
  else
    return  0
  end
end

return { run=run, output=outputs }