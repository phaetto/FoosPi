namespace Services.RaspberryPi.GpIo.Debug
{
    using System;
    using System.Windows.Forms;

    public partial class IRBridgeForm : Form
    {
        private int count = 10;

        public IRBridgeForm()
        {
            InitializeComponent();
        }

        private void IRBridgeForm_Load(object sender, EventArgs e)
        {
            for (var n = 1; n < count; ++n)
            {
                performanceChart.Series[0].Points.AddXY(n, 0);
            }
        }

        public void PushToChart(bool value)
        {
            if (performanceChart != null && performanceChart.Series.Count > 0)
            {
                performanceChart.Series[0].Points.AddXY(count, value ? 1 : 0);

                if (performanceChart.Series[0].Points.Count > 40)
                {
                    for (var i = 0; i < 20; ++i)
                    {
                        performanceChart.Series[0].Points.RemoveAt(0);
                    }
                }

                ++count;
            }
        }
    }
}
