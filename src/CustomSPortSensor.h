#ifndef CUSTOMSPORTSENSOR_H
    #define CUSTOMSPORTSENSOR_H

    #include <SPort.h>

    class CustomSPortSensor : public SPortSensor {
        public:
            CustomSPortSensor(sportData (*callback)(CustomSPortSensor*));
            virtual sportData getData ();
        private:
            sportData (*_callback)(CustomSPortSensor*);
    };

#endif