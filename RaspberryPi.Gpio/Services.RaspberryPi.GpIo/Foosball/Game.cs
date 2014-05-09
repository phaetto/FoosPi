namespace Services.RaspberryPi.GpIo.Foosball
{
    using Chains;

    public sealed class Game : Chain<Game>
    {
        public int BlueScore { get; set; }

        public int RedScore { get; set; }
    }
}
