namespace i2c {
    namespace hw {

        class Slave: public applicationEvents::EventHandler {

        protected:
            target::i2c::Peripheral* peripheral;            
            int index;
            int afterWriteEventId;

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

                afterWriteEventId = applicationEvents::createEventId();
                handle(afterWriteEventId);

/*                
                peripheral->TIMINGR.setPRESC(1);
                peripheral->TIMINGR.setSCLL(0xC7);
                peripheral->TIMINGR.setSCLH(0xC3);
                peripheral->TIMINGR.setSDADEL(0x02);
                peripheral->TIMINGR.setSCLDEL(0x04);
                peripheral->CR2.setAUTOEND(1);
                peripheral->CR1.setRXIE(1);
                peripheral->CR1.setTXIE(1);
                peripheral->CR1.setSTOPIE(1);
                peripheral->CR1.setPE(1);
                stopEventId = applicationEvents::createEventId();
                handle(stopEventId);
                */
            }
            
            virtual void onEvent() {
                afterWrite();
            }

            void handleInterrupt() {

                if (peripheral->ISR.getADDR()) {
                    peripheral->ICR.setADDRCF(1);
                    index = 0;
                    peripheral->ISR.setTXE(1); // clear TX buffer
                }

                if (peripheral->ISR.getRXNE()) {
                    int byte = peripheral->RXDR.getRXDATA();
                    onRx(byte, index++);
                }

                if (peripheral->ISR.getTXE()) {                                        
                    peripheral->TXDR.setTXDATA(onTx(index));
                    index++;
                }

                if (peripheral->ISR.getSTOPF()) {				
                    peripheral->ICR.setSTOPCF(1);
                    if (!peripheral->CR2.getRD_WRN()) {
                        applicationEvents::schedule(afterWriteEventId);
                    }                    
                }                
               
            }
            
            virtual void onRx(int byte, int index) {
            }

            virtual int onTx(int index) {
                return 0;
            }

            virtual void afterWrite() {
            }

        };
    }
}