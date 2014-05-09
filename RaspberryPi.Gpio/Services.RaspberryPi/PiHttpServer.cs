namespace Services.RaspberryPi
{
    using System.Linq;
    using System.Net;
    using Chains;
    using Chains.Play.Web;
    using Chains.Play.Web.HttpListener;
    using Services.Management.Administration.Worker;
    using Services.RaspberryPi.Handlers;

    public sealed class PiHttpServer : Chain<PiHttpServer>
    {
        private readonly WorkUnitContext workUnitContext;

        private readonly HttpServer httpServer;

        public PiHttpServer(WorkUnitContext workUnitContext)
        {
            this.workUnitContext = workUnitContext;

            workUnitContext.LogLine("Starting server at ip {0}", GetLocalNetworkIp());

            httpServer = new HttpServer(new ServerHost(GetLocalNetworkIp(), 18080), new string[0], maxThreads: 3);
            new FileSystemHandler(httpServer);
        }

        private string GetLocalNetworkIp()
        {
            var localIp = "?";
            var host = Dns.GetHostEntry(Dns.GetHostName());
            foreach (var ip in host.AddressList.Where(ip => ip.AddressFamily.ToString() == "InterNetwork"))
            {
                localIp = ip.ToString();
            }

            return localIp;
        }
    }
}
