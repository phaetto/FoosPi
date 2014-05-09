namespace Services.RaspberryPi.GpIo.Actions
{
    using Chains;
    using Chains.Play;
    using RaspberryPiDotNet;

    public sealed class SetGpioValue : ReproducibleWithData<SetGpioValueData>, IChainableAction<GpioContext, GpioContext>
    {
        public SetGpioValue(SetGpioValueData data)
            : base(data)
        {
        }

        public GpioContext Act(GpioContext context)
        {
            using (GPIO gpio = new GPIOMem(Data.Pin, GPIODirection.Out))
            {
                gpio.Write(Data.ValueToSet);
            }

            return context;
        }
    }
}
