namespace Services.RaspberryPi.GpIo.Actions
{
    using System;
    using Chains.Play;
    using RaspberryPiDotNet;

    [Serializable]
    public sealed class SetGpioValueData : SerializableSpecification
    {
        public GPIOPins Pin;

        public bool ValueToSet;

        public override int DataStructureVersionNumber
        {
            get
            {
                return 1;
            }
        }
    }
}
