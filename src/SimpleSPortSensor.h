#ifndef SIMPLESPORTSENSOR_H
    #define SIMPLESPORTSENSOR_H

    #include <SPort.h>
    
    class SimpleSPortSensor : public SPortSensor {
        public: 
            SimpleSPortSensor(int id);
            sportData getData ();
            long value;
        private:
            int _id;
    };

#endif