namespace i2c {
    namespace hw {

        class Slave {

        protected:
            target::i2c::Peripheral* peripheral;            
            int indexRx;
            int indexTx;

        public:
            
            void init(target::i2c::Peripheral* peripheral, int address) {
                this->peripheral = peripheral;

                peripheral->OAR1.setOA1_1(address);
                peripheral->OAR1.setOA1EN(true);

                peripheral->CR1.setRXIE(1);
                peripheral->CR1.setTXIE(1);
                peripheral->CR1.setSTOPIE(1);
                peripheral->CR1.setADDRIE(1);
                peripheral->CR1.setPE(1);
            }
            


            void handleInterrupt() {

                if (peripheral->ISR.getADDR()) {
                    peripheral->ICR.setADDRCF(1);
                    indexRx = 0;
                    indexTx = 0;
                    peripheral->ISR.setTXE(1); // clear TX buffer
                }

                if (peripheral->ISR.getRXNE()) {
                    int byte = peripheral->RXDR.getRXDATA();
                    onRx(byte, indexRx++);
                }

                if (peripheral->ISR.getTXE()) {                                        
                    peripheral->TXDR.setTXDATA(onTx(indexTx++));
                }

                if (peripheral->ISR.getSTOPF()) {				
                    peripheral->ICR.setSTOPCF(1);
                    if (!peripheral->ISR.getDIR()) {
                        onStop(peripheral->ISR.getDIR());
                    }                    
                }                
               
            }
            
            virtual void onRx(int byte, int index) {
            }

            virtual int onTx(int index) {
                return 0;
            }

            virtual void onStop(bool read) {
            }

        };

        class BufferedSlave: public Slave {

        protected:
            unsigned char* rxBuffer;
            unsigned char* txBuffer;
            int rxSize;
            int txSize;

        public:

            void init(target::i2c::Peripheral* peripheral, int address, unsigned char* rxBuffer, int rxSize, unsigned char* txBuffer, int txSize) {
                Slave::init(peripheral, address);
                this->rxBuffer = rxBuffer;
                this->rxSize = rxSize;
                this->txBuffer = txBuffer;
                this->txSize = txSize;
            }

            virtual void onRx(int byte, int index) {
                if (index < rxSize) {
                    rxBuffer[index] = byte;
                }
                if (index == rxSize - 1) {
                    onRxComplete();
                }
            }

            virtual int onTx(int index) {
                return index < txSize? txBuffer[index]: 0;
                if (index == txSize - 1) {
                    onTxComplete();
                }
            }

            virtual void onRxComplete() {                
            }

            virtual void onTxComplete() {                
            }
        };
    }
}