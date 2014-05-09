namespace Services.RaspberryPi.GpIo.Actions
{
    using System;
    using Chains.Play;

    [Serializable]
    public sealed class GetGpioValueReturnData : SerializableSpecification
    {
        public bool Value;

        public override int DataStructureVersionNumber
        {
            get
            {
                return 1;
            }
        }
    }
}
