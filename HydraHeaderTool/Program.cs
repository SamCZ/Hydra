using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace HydraHeaderTool
{
    class Program
    {
        static void Main(string[] args)
        {
            string generatedHeadFilesFolder = "GeneratedHeaders";
            string hydraFolder = "Hydra";

            if(!Directory.Exists(generatedHeadFilesFolder))
            {
                Directory.CreateDirectory(generatedHeadFilesFolder);
            }

            foreach (string dir in Directory.GetFiles(hydraFolder, "*", SearchOption.AllDirectories))
            {

            }
        }
    }
}
