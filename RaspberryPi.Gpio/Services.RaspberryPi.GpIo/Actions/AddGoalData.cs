namespace Services.RaspberryPi.GpIo.Actions
{
    using Chains.Play;

    public class AddGoalData : SerializableSpecification
    {
        public int ChannelActivated;

        public int McuSpeed;

        public override int DataStructureVersionNumber
        {
            get
            {
                return 0;
            }
        }
    }
}
