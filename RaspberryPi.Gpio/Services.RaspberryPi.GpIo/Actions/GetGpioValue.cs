namespace Services.RaspberryPi.GpIo.Actions
{
    using Chains.Play;
    using RaspberryPiDotNet;

    public sealed class GetGpioValue : RemotableActionWithData<GetGpioValueData, GetGpioValueReturnData, GpioContext>
    {
        public GetGpioValue(GetGpioValueData data)
            : base(data)
        {
        }

        protected override GetGpioValueReturnData ActRemotely(GpioContext context)
        {
            using (GPIO gpio = new GPIOMem(Data.Pin, GPIODirection.Out, Data.DefaultValue))
            {
                return new GetGpioValueReturnData
                       {
                           Value = gpio.Read(),
                       };
            }
        }
    }
}
