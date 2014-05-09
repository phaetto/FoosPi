namespace Services.RaspberryPi.GpIo.Communication
{
    using System.Threading;
    using RaspberryPiDotNet;

    public sealed class McuSerial
    {
        private const int ThreadSleepMilliseconds = 1;

        private readonly GPIOPins clockPin;

        private readonly GPIOPins ioPin;

        private bool clockValue;

        private bool isSynced = false;

        public McuSerial(GPIOPins clockPin, GPIOPins ioPin, bool clockStartValue = false)
        {
            this.clockPin = clockPin;
            this.ioPin = ioPin;
            this.clockValue = clockStartValue;

            GPIOMem.Write(clockPin, clockValue);
            GPIOMem.Write(ioPin, false);
        }

        public void Sync(int skipBits)
        {
            if (isSynced)
            {
                return;
            }

            GPIOMem.Write(ioPin, false);
            for (var n = 0; n < skipBits; ++n)
            {
                FullClock();
            }

            GPIOMem.Write(ioPin, true);
            FullClock();

            isSynced = true;
        }

        public void ClockValueWrite(bool value)
        {
            isSynced = false;

            GPIOMem.Write(ioPin, value);
            FullClock();
        }

        public uint ClockValueRead()
        {
            isSynced = false;

            HalfClock();
            var value = GPIOMem.Read(ioPin);
            HalfClock();

            return value ? 1u : 0u;
        }

        public bool GetAck()
        {
            if (ClockValueRead() == 1)
            {
                return true;
            }

            // Ack failed
            isSynced = false;
            return false;
        }

        public void SendAck()
        {
            ClockValueWrite(true);
        }

        private void HalfClock()
        {
            clockValue = !clockValue;
            GPIOMem.Write(clockPin, clockValue);
            ProtocolSleep();
        }

        private void FullClock()
        {
            clockValue = !clockValue;
            GPIOMem.Write(clockPin, clockValue);
            ProtocolSleep();

            clockValue = !clockValue;
            GPIOMem.Write(clockPin, clockValue);
            ProtocolSleep();
        }

        private void ProtocolSleep()
        {
            Thread.Sleep(ThreadSleepMilliseconds);

            //var counter = 100000;
            //while (--counter > 0);
        }
    }
}
