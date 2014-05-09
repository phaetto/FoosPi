namespace Services.RaspberryPi.GpIo
{
    using System;

    [Flags]
    public enum FoosPiNumberBoardCommands : byte
    {
        NoCommand = 0x00,
        GoalSignCommand = 0x08,
        GoalBlueCommand = 0x04,
        ResetGame = 0x01,
    }
}
