namespace i2c {
namespace hw {

class Slave {

protected:
  volatile target::i2c::Peripheral *peripheral;
  int indexRx;
  int indexTx;

public:
  void init(volatile target::i2c::Peripheral *peripheral, int address) {
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

      indexRx = 0;
      indexTx = 0;
      peripheral->ISR.setTXE(1); // clear TX buffer
      peripheral->ICR.setADDRCF(1);

    } else if (peripheral->ISR.getRXNE()) {

      int byte = peripheral->RXDR.getRXDATA();
      onRx(byte, indexRx++);

    } else if (peripheral->ISR.getTXE()) {

      peripheral->TXDR.setTXDATA(onTx(indexTx++));

    } else if (peripheral->ISR.getSTOPF()) {

      if (!peripheral->ISR.getDIR()) {
        onStop(peripheral->ISR.getDIR());
      }
      peripheral->ICR.setSTOPCF(1);
    }
  }

  virtual void onRx(int byte, int index) {}

  virtual int onTx(int index) { return 0; }

  virtual void onStop(bool read) {}
};

class BufferedSlave : public Slave {

protected:
  unsigned char *rxBuffer;
  unsigned char *txBuffer;
  int rxSize;
  int txSize;

public:
  void init(volatile target::i2c::Peripheral *peripheral, int address,
            unsigned char *rxBuffer, int rxSize, unsigned char *txBuffer,
            int txSize) {
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
    if (index == 0) {
      onTxStart();
    }

    if (index == txSize - 1) {
      onTxComplete();
    }

    return index < txSize ? txBuffer[index] : 0;
  }

  virtual void onRxComplete() {}

  virtual void onTxStart() {}

  virtual void onTxComplete() {}
};
} // namespace hw
} // namespace i2c