namespace Services.RaspberryPi.GpIo
{
    using System;
    using System.Threading;
    using Chains;
    using RaspberryPiDotNet;
    using Services.Management.Administration.Worker;
    using Services.RaspberryPi.GpIo.Communication;

    public sealed class GpioGoalCheckBridgeContext : Chain<GpioGoalCheckBridgeContext>
    {
        private readonly WorkUnitContext workUnitContext;

        private readonly McuSerial mcuSerial = new McuSerial(GPIOPins.V2_GPIO_22, GPIOPins.V2_GPIO_27);

        public GpioGoalCheckBridgeContext(WorkUnitContext workUnitContext)
        {
            this.workUnitContext = workUnitContext;
            new Thread(ProtoPiCommunicationThread).Start();
        }

        private void ProtoPiCommunicationThread()
        {
            while (true)
            {
                CommunicateWithMCU();

                Thread.Sleep(10);
            }
        }

        private void CommunicateWithMCU()
        {
            // Get the channel ready
            mcuSerial.Sync(34);

            // Receive 8bits - channel
            uint channel = 0x0;
            for (var n = 0; n < 8; ++n)
            {
                channel |= (mcuSerial.ClockValueRead()<<n);
            }

            // Read 1-bit ack
            if (!mcuSerial.GetAck())
            {
                // Ack failed
                return;
            }

            // Receive 16bits - timer response
            uint timer = 0x0;
            for (var n = 0; n < 24; ++n)
            {
                timer |= (mcuSerial.ClockValueRead() << n);
            }

            // Send 1-bit ack - end the communication
            mcuSerial.SendAck();

            workUnitContext.LogLine("Got back: Channel: {0:x}, Clock: {1:x}", channel, timer);
        }

        public void OnStart()
        {
        }

        public void OnStop()
        {
            // Bring to high-impendance; does not block programmer with external voltages
            GPIOMem.Read(GPIOPins.V2_GPIO_22);
            GPIOMem.Read(GPIOPins.V2_GPIO_27);
        }
    }
}
