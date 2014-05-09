namespace Services.RaspberryPi.GpIo.Debug
{
    using System.Threading;
    using System.Windows.Forms;
    using Chains;
    using Chains.Play.Web;
    using RaspberryPiDotNet;
    using Services.Communication.Protocol;
    using Services.Management.Administration.Worker;
    using Services.RaspberryPi.GpIo.Actions;

    public sealed class GpioTest : Chain<GpioTest>
    {
        private readonly string contextServer;

        private readonly int contextPort;

        private readonly WorkUnitContext workUnitContext;

        private static IRBridgeForm irBridgeForm;

        public GpioTest(string contextServer, int contextPort, WorkUnitContext workUnitContext)
        {
            this.contextServer = contextServer;
            this.contextPort = contextPort;
            this.workUnitContext = workUnitContext;
            new Thread(StartDoingMyStuff).Start();

            Application.EnableVisualStyles();
            Application.SetCompatibleTextRenderingDefault(false);
            irBridgeForm = new IRBridgeForm();
            Application.Run(irBridgeForm);
        }

        private void StartDoingMyStuff()
        {
            using (var connection = new Client(contextServer, contextPort).Do(new OpenConnection()))
            {
                while (true)
                {
                    IRBridge(connection, workUnitContext, true);
                    Thread.Sleep(100);
                }
            }
        } 

        private static void IRBridge(ClientConnectionContext clientConnectionContext, WorkUnitContext workUnitContext, bool transmitterOn)
        {
            var isOn = clientConnectionContext.Do(
                new Send<GetGpioValueReturnData>(
                    new GetGpioValue(
                        new GetGpioValueData
                        {
                            DefaultValue = false,
                            Pin = GPIOPins.V2_Pin_P1_10,
                        }))).Value;

            irBridgeForm.PushToChart(isOn);
        }

        //private static int AdcValueRead(WorkUnitContext workUnitContext)
        //{
        //    // Refactor this as a chain for MCP3002

        //    // OUT / GPIO10 : DIN       [Configuration bits]
        //    // OUT / GPIO11 : CLK       [Clock] 0/1 - rp sets it
        //    // OUT / GPIO7 : CS/SHDN    [Control bit] (0 - working, 1 - sleep)
        //    // IN / GPIO9 : DOUT        [Output bits]

        //    // 4 bits config + null bit + output (10 bits)

        //    var response = 0;

        //    workUnitContext.CanStop = false;

        //    var clockBit = true;

        //    GPIO controlGpio = new GPIOMem(GPIOPins.GPIO_07, GPIODirection.Out, true);
        //    GPIO clockGpio = new GPIOMem(GPIOPins.GPIO_11, GPIODirection.Out, clockBit);
        //    GPIO configurationGpio = new GPIOMem(GPIOPins.GPIO_10, GPIODirection.Out, true);
        //    GPIO outputGpio = new GPIOMem(GPIOPins.GPIO_09, GPIODirection.In, false);

        //    // Enable control bit (set to false)
        //    controlGpio.Write(false);
        //    configurationGpio.Write(true);

        //    // 4 config options are 1 bit
        //    for (var i = 0; i < 4; ++i)
        //    {
        //        clockBit = MoveHalfClock(clockGpio, clockBit);
        //        clockBit = MoveHalfClock(clockGpio, clockBit);
        //    }

        //    // Null bit
        //    clockBit = MoveHalfClock(clockGpio, clockBit);
        //    clockBit = MoveHalfClock(clockGpio, clockBit);
            
        //    // Multiplexed output
        //    for (var i = 0; i < 10; ++i)
        //    {
        //        clockBit = MoveHalfClock(clockGpio, clockBit);

        //        var responseBit = outputGpio.Read();
        //        response = (response << 1) | (responseBit ? 1 : 0);

        //        clockBit = MoveHalfClock(clockGpio, clockBit);
        //    }

        //    controlGpio.Write(true);

        //    controlGpio.Dispose();
        //    clockGpio.Dispose();
        //    configurationGpio.Dispose();
        //    outputGpio.Dispose();

        //    workUnitContext.CanStop = true;

        //    return response;
        //}

        //private static bool MoveHalfClock(GPIO clockGpio, bool clockBit)
        //{
        //    clockGpio.Write(clockBit = !clockBit);
        //    Thread.Sleep(1);
        //    return clockBit;
        //}

        //private static float CalculateTMP36FromADCOutput(int response)
        //{
        //    return (float)((3.3 * response / 0x3ff) * 100 - 50);
        //}
    }
}
