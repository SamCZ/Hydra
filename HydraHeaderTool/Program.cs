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
        private static string GeneratedHeadFilesFolder = "GeneratedHeaders";
        private static string HydraFolder = "Hydra";

        static void Main(string[] args)
        {
            

            if(!Directory.Exists(GeneratedHeadFilesFolder))
            {
                Directory.CreateDirectory(GeneratedHeadFilesFolder);
            }

            foreach (string file in Directory.GetFiles(HydraFolder, "*", SearchOption.AllDirectories))
            {
                string extension = Path.GetExtension(file);
                
                if (extension == ".h")
                {
                    ReadHeaderFile(file);
                }
            }
        }

        private static void ReadHeaderFile(string file)
        {
            string[] lines = File.ReadAllLines(file, Encoding.UTF8);

            string className = string.Empty;

            for (int i = 0; i < lines.Length; i++)
            {
                string line = lines[i];

                if(line.StartsWith("HCLASS"))
                {
                    string nextLine = lines[i + 1];

                    string[] spl = nextLine.Split(' ');

                    int classNameIndex = 1;

                    foreach(string cspl in spl)
                    {
                        if(cspl.Equals("HYDRA_API"))
                        {
                            classNameIndex = 2;
                            break;
                        }
                    }

                    if(classNameIndex <= spl.Length - 1)
                    {
                        className = spl[classNameIndex];
                    }
                }
            }

            if(className.Length > 0)
            {
                InsertHeaderIncludeIfNotExist(file, lines, className);

                string clsnUpper = className.ToUpper();

                string EOL = "\r\n";
                string generatedString = string.Empty;

                string globalDefName = "HCLASS_DEF_" + className + "_generated_h";

                generatedString += "#ifdef " + globalDefName + EOL;
                generatedString += "#error \"" + className + ".generated.h already included, missing '#pragma once' in " + className + ".h" + "\"" + EOL;
                generatedString += "#endif" + EOL;

                generatedString += "#define " + globalDefName + EOL + EOL;

                generatedString += "#define HCLASS_GEN_" + clsnUpper + " \\" + EOL;
                generatedString += "protected: \\" + EOL;
                generatedString += "    static HObject* Factory_" + className + "() { return new " + className + "(); } \\" + EOL;
                generatedString += "public: \\" + EOL;
                generatedString += "    static HClass StaticClass() { return HClass(\"" + className + "\", " + className + "::Factory_" + className + "); } \\" + EOL;
                generatedString += "    virtual HClass GetClass() const { return HClass(\"" + className + "\", " + className + "::Factory_" + className + "); }" + EOL;

                generatedString += EOL;

                generatedString += "#undef HCLASS_GENERATED_BODY" + EOL;
                generatedString += "#define HCLASS_GENERATED_BODY(...) HCLASS_GEN_" + clsnUpper + EOL;
                //generatedString += "" + EOL;

                File.WriteAllText(Path.Combine(GeneratedHeadFilesFolder, Path.GetFileNameWithoutExtension(file) + ".generated.h"), generatedString);
            }
        }

        private static void InsertHeaderIncludeIfNotExist(string file, string[] lines, string className)
        {
            //List<string> newLines = new List<string>();
            
            // TODO: Auto include

            //File.WriteAllLines(file, newLines);
        }
    }
}
