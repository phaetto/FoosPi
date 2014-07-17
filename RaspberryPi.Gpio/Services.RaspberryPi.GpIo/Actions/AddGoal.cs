namespace Services.RaspberryPi.GpIo.Actions
{
    using Chains;
    using Chains.Play;

    public sealed class AddGoal : ReproducibleWithData<AddGoalData>, IChainableAction<Gpio4X7SegmentLedContext, Gpio4X7SegmentLedContext>
    {
        public AddGoal(AddGoalData data)
            : base(data)
        {
        }

        public Gpio4X7SegmentLedContext Act(Gpio4X7SegmentLedContext context)
        {
            if (Data.ChannelActivated == 1)
            {
                context.WorkUnitContext.LogLine("GOAL -- RED");
                context.RedScore++;
            }
            else if (Data.ChannelActivated == 2)
            {
                context.WorkUnitContext.LogLine("GOAL -- BLUE");
                context.BlueScore++;
            }

            if (context.RedScore > 10 || context.BlueScore > 10)
            {
                // A win action must be invoked here, probably with some effects.

                context.RedScore = 0;
                context.BlueScore = 0;
            }

            return context;
        }
    }
}
