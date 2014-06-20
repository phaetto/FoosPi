namespace Services.RaspberryPi.GpIo
{
    using System;

    [Flags]
    public enum FoosPiNumberBoardCommands : byte
    {
        NoCommand = 0x00,
        GoalSignCommandBit = 0x08,
        GoalRedCommand = 0x01,
        GoalBlueCommand = 0x02,
        ZeroCommand = 0x03,
        ResetCommand = 0x04,
    }
}
