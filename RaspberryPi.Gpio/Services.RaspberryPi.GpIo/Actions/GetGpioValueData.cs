namespace Services.RaspberryPi.GpIo.Actions
{
    using System;
    using Chains.Play;
    using RaspberryPiDotNet;

    [Serializable]
    public sealed class GetGpioValueData : SerializableSpecification
    {
        public GPIOPins Pin;

        public bool DefaultValue;

        public override int DataStructureVersionNumber
        {
            get
            {
                return 1;
            }
        }
    }
}
