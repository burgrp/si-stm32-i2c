namespace i2c {
    namespace hw {

        class Master: public applicationEvents::EventHandler {

        protected:
            target::i2c::Peripheral* peripheral;
            int length = 0;
            int index = 0;
            int stopEventId;

            void start(int address, int length, int rdWrn) {
                this->length = length;
                this->index = 0;
                peripheral->ICR.setNACKCF(1);
                peripheral->CR2.setSADD(address << 1);
                peripheral->CR2.setNBYTES(length);
                peripheral->CR2.setRD_WRN(rdWrn);
                peripheral->CR2.setSTART(1);
            }

        public:
            
            void init(target::i2c::Peripheral* peripheral) {
                this->peripheral = peripheral;
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
            }
            
            virtual void onEvent() {
                onStop(peripheral->CR2.getRD_WRN(), peripheral->ISR.getNACKF());
            }

            void handleInterrupt() {

                if (peripheral->ISR.getRXNE()) {
                    int byte = this->peripheral->RXDR.getRXDATA();
                    onRx(byte, this->index++);
                }

                if (peripheral->ISR.getTXIS()) {
                    peripheral->TXDR.setTXDATA(onTx(index));
                }

                if (peripheral->ISR.getSTOPF()) {				
                    peripheral->ICR.setSTOPCF(1);
                    applicationEvents::schedule(stopEventId);
                }
                
            }
            
            virtual void onRx(int byte, int index) {
            }

            virtual int onTx(int index) {
                return 0;
            }

            virtual void onStop(bool read, int error) {
            }

            void read(int address, int length) {
                this->start(address, length, 1);
            }

            void write(int address, int length) {
                this->start(address, length, 0);
            }
        };

    }
}