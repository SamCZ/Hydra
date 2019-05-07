using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Security.Cryptography;
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
            bool haveInclude = false;
            int lastIncludeLineIndex = -1;

            for (int i = 0; i < lines.Length; i++)
            {
                string line = lines[i];

                if(line.StartsWith("class") || line.StartsWith("struct") || line.StartsWith("enum") || line.StartsWith("namespace"))
                {
                    if(lastIncludeLineIndex == -1)
                    {
                        lastIncludeLineIndex = i - 1;
                    }
                } else if(line.StartsWith("HCLASS"))
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
                } else if(line.StartsWith("#include")) {
                    haveInclude |= line.Contains("generated");
                    lastIncludeLineIndex = i;
                }
            }

            if(className.Length > 0)
            {
                string generatedFileName = Path.GetFileNameWithoutExtension(file) + ".generated.h";
                string generatedFilePath = Path.Combine(GeneratedHeadFilesFolder, generatedFileName);

                if (!haveInclude)
                {
                    InsertHeaderIncludeIfNotExist(file, lines, className, lastIncludeLineIndex, generatedFileName);
                }

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

                if(File.Exists(generatedFilePath))
                {
                    if(CalculateMD5(generatedFilePath) == CalculateMD5FromMemory(generatedString))
                    {
                        return;
                    }
                }

                File.WriteAllText(generatedFilePath, generatedString);
            }
        }

        static string CalculateMD5(string filename)
        {
            using (var md5 = MD5.Create())
            {
                using (var stream = File.OpenRead(filename))
                {
                    var hash = md5.ComputeHash(stream);
                    return BitConverter.ToString(hash).Replace("-", "").ToLowerInvariant();
                }
            }
        }

        static string CalculateMD5FromMemory(string data)
        {
            using (var md5 = MD5.Create())
            {
                byte[] byteArray = Encoding.ASCII.GetBytes(data);
                using (var stream = new MemoryStream(byteArray))
                {
                    var hash = md5.ComputeHash(stream);
                    return BitConverter.ToString(hash).Replace("-", "").ToLowerInvariant();
                }
            }
        }

        private static void InsertHeaderIncludeIfNotExist(string file, string[] lines, string className, int lastIncludeIndex, string generatedFileName)
        {
            List<string> newLines = new List<string>();

            for (int i = 0; i < lines.Length; i++)
            {
                string line = lines[i];

                newLines.Add(line);

                if(i == lastIncludeIndex)
                {
                    newLines.Add("#include \"" + generatedFileName + "\"\r\n");
                }
            }

            File.WriteAllLines(file, newLines);
        }
    }
}
