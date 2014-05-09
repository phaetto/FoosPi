﻿namespace Services.RaspberryPi.GpIo
{
    using System;
    using System.Threading;
    using Chains;
    using RaspberryPiDotNet;
    using Services.Management.Administration.Worker;
    using Services.RaspberryPi.GpIo.Communication;

    public sealed class Gpio4X7SegmentLedContext : Chain<Gpio4X7SegmentLedContext>, IWorkerEvents
    {
        private readonly WorkUnitContext workUnitContext;

        private readonly McuSerial mcuSerial = new McuSerial(GPIOPins.V2_GPIO_24, GPIOPins.V2_GPIO_23);

        public int BlueScore = 34;

        public int RedScore = 12;

        public Gpio4X7SegmentLedContext(WorkUnitContext workUnitContext)
        {
            this.workUnitContext = workUnitContext;
            new Thread(BoardCommunicationThread).Start();
        }

        private void BoardCommunicationThread()
        {
            while (true)
            {
                CommunicateWithBoard();

                Thread.Sleep(10);
            }
        }

        private void CommunicateWithBoard()
        {
            // Get the channel ready
            mcuSerial.Sync(26);

            // Receive 8bits - command
            uint command = 0x0;
            for (var n = 0; n < 8; ++n)
            {
                command |= (mcuSerial.ClockValueRead() << n);
            }

            // Read 1-bit ack
            if (!mcuSerial.GetAck())
            {
                // Ack failed
                return;
            }

            // Parse command
            OnCommand((FoosPiNumberBoardCommands)command);
            
            var valuesToSend = new[]
                               {
                                   GetTensOfNumber(RedScore),
                                   GetLowerDigit(RedScore),
                                   GetTensOfNumber(BlueScore),
                                   GetLowerDigit(BlueScore),
                               };

            // Send 4x4bits - score updated
            for (var n = 0; n < valuesToSend.Length; ++n)
            {
                for (var i = 0; i < 4; ++i)
                {
                    var po2 = (1<<i);
                    mcuSerial.ClockValueWrite((valuesToSend[n] & po2) == po2);
                }
            }

            // Send 1-bit ack - end the communication
            mcuSerial.SendAck();
        }

        private void OnCommand(FoosPiNumberBoardCommands command)
        {
            if (command == FoosPiNumberBoardCommands.NoCommand)
            {
                return;
            }

            var goalNumberRequested = ((int)command & 0xF0) >> 4;

            if (HasFlag(command, FoosPiNumberBoardCommands.GoalSignCommand))
            {
                goalNumberRequested = -goalNumberRequested;
            }

            if (goalNumberRequested != 0)
            {
                if (HasFlag(command, FoosPiNumberBoardCommands.GoalBlueCommand))
                {
                    BlueScore += goalNumberRequested;

                    workUnitContext.LogLine("Command received: BLUE: 0x{0:x}", command);
                }
                else
                {
                    RedScore += goalNumberRequested;

                    workUnitContext.LogLine("Command received: RED: 0x{0:x}", command);
                }
            }

            // Change with Game here

            if (HasFlag(command, FoosPiNumberBoardCommands.ResetGame))
            {
                BlueScore = RedScore = 0;

                workUnitContext.LogLine("Command received: RESET: 0x{0:x}", command);
            }
        }

        private static bool HasFlag(FoosPiNumberBoardCommands command, FoosPiNumberBoardCommands flag)
        {
            return ((command & flag) == flag);
        }

        private static int GetTensOfNumber(int number)
        {
            return (int)Math.Floor((float)number / 10);
        }

        private static int GetLowerDigit(int number)
        {
            return number - 10 * GetTensOfNumber(number);
        }

        public void OnStart()
        {
        }

        public void OnStop()
        {
            // Bring to high-impendance; does not block programmer with external voltages
            GPIOMem.Read(GPIOPins.V2_GPIO_23);
            GPIOMem.Read(GPIOPins.V2_GPIO_24);
        }
    }
}