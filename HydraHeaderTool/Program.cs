using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Security.Cryptography;
using System.Text;
using System.Threading.Tasks;

namespace HydraHeaderTool
{
    public class HClass
    {
        public string Name;
        public string InheritedClassName;
        
        public HClass InheritedClass = null;

        public List<HClass> Hiearchy;

        public HClass(string name, string inheritedClassName)
        {
            Name = name;
            InheritedClassName = inheritedClassName;
        }

        public void BuildHiearchy()
        {
            Hiearchy = new List<HClass>();

            HClass clazz = InheritedClass;

            while(clazz != null)
            {
                Hiearchy.Add(clazz);

                clazz = clazz.InheritedClass;
            }

            if(InheritedClass == null && InheritedClassName.Length > 0)
            {
                Hiearchy.Add(new HClass(InheritedClassName, ""));
            }
        }
    }

    class Program
    {
        private static string GeneratedHeadFilesFolder = ".\\GeneratedHeaders";
        private static string SourceFolder = ".\\";

        private static string EOL = System.Environment.NewLine;

        private static bool IsEngineFolder = false;

        private static int GeneratedFilesCount = 0;

        static List<HClass> ClassDatabase = new List<HClass>();

        static void Main(string[] args)
        {
            if(!Directory.Exists(GeneratedHeadFilesFolder))
            {
                Directory.CreateDirectory(GeneratedHeadFilesFolder);
            }

            IsEngineFolder = Path.GetFileNameWithoutExtension(Directory.GetCurrentDirectory()) == "Hydra";

            // Generate headers

            foreach (string file in Directory.GetFiles(SourceFolder, "*", SearchOption.AllDirectories))
            {
                string extension = Path.GetExtension(file);
                
                if (extension == ".h")
                {
                    ReadHeaderFile(file);
                }
            }

            // Generate class database

            foreach(HClass class0 in ClassDatabase)
            {
                foreach (HClass class1 in ClassDatabase)
                {
                    if(class0.InheritedClassName == class1.Name)
                    {
                        class0.InheritedClass = class1;
                    }
                }
            }

            string classDatabaseFileData = string.Empty;

            classDatabaseFileData += "#pragma once" + EOL + EOL;
            classDatabaseFileData += "#include \"Hydra/Framework/Class.h\"" + EOL + EOL;

            if(IsEngineFolder)
            {
                classDatabaseFileData += "static inline void Hydra_InitializeClassDatabase()" + EOL;
            } else
            {
                classDatabaseFileData += "static inline void Game_InitializeClassDatabase()" + EOL;
            }

            classDatabaseFileData += "{" + EOL;

            
            foreach (HClass class0 in ClassDatabase)
            {
                class0.BuildHiearchy();

                string dbClassStr = "   HClassDatabase::Add(" + "\"" + class0.Name + "\"" + ", {";
                
                foreach (HClass class1 in class0.Hiearchy)
                {
                    dbClassStr += "\"" + class1.Name + "\"" + ",";
                }

                if(class0.Hiearchy.Count > 0)
                {
                    dbClassStr = dbClassStr.Substring(0, dbClassStr.Length - 1);
                }

                dbClassStr += "});";

                classDatabaseFileData += dbClassStr + EOL;
            }

            classDatabaseFileData += "}" + EOL;

            string classDatabaseFilePath;

            if (IsEngineFolder)
            {
                classDatabaseFilePath = Path.Combine(GeneratedHeadFilesFolder, "HydraClassDatabase.generated.h");
            } else
            {
                classDatabaseFilePath = Path.Combine(GeneratedHeadFilesFolder, "GameClassDatabase.generated.h");
            }

            if (File.Exists(classDatabaseFilePath))
            {
                if (CalculateMD5(classDatabaseFilePath) == CalculateMD5FromMemory(classDatabaseFileData))
                {
                    return;
                }
            }

            File.WriteAllText(classDatabaseFilePath, classDatabaseFileData);

            Console.WriteLine("HydraHeaderTool: Generated " + GeneratedFilesCount + " files.");
        }

        private static void ReadHeaderFile(string file)
        {
            string[] lines = File.ReadAllLines(file, Encoding.UTF8);

            string className = string.Empty;
            string parentClassName = string.Empty;
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
                    int dividerIndex = 0;

                    foreach(string cspl in spl)
                    {
                        if(cspl.Equals("HYDRA_API"))
                        {
                            classNameIndex = 2;
                        }

                        if(!cspl.Equals(":"))
                        {
                            dividerIndex++;
                        } else
                        {
                            break;
                        }
                    }

                    if(classNameIndex <= spl.Length - 1)
                    {
                        className = spl[classNameIndex];
                    }

                    if(dividerIndex > 0)
                    {
                        for(int di = dividerIndex + 1; di < spl.Length; di++)
                        {
                            if (spl[di].Equals("public")) continue;
                            
                            if(spl[di].Contains(","))
                            {
                                string[] parentClassesSplit = spl[di].Split(',');

                                if(parentClassesSplit.Length > 0)
                                {
                                    parentClassName = parentClassesSplit[0].Replace(' ', '\0');
                                } else
                                {
                                    break;
                                }
                            } else
                            {
                                parentClassName = spl[di];
                            }
                        }
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
                
                ClassDatabase.Add(new HClass(className, parentClassName));

                if (!haveInclude)
                {
                    InsertHeaderIncludeIfNotExist(file, lines, className, lastIncludeLineIndex, generatedFileName);
                }

                string clsnUpper = className.ToUpper();

                
                string generatedString = string.Empty;

                string globalDefName = "HCLASS_DEF_" + className + "_generated_h";

                if(IsEngineFolder)
                {
                    generatedString += "#include \"HydraClassDatabase.generated.h\"" + EOL + EOL;
                } else
                {
                    generatedString += "#include \"GameClassDatabase.generated.h\"" + EOL + EOL;
                }

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
                
                if (File.Exists(generatedFilePath))
                {
                    if(CalculateMD5(generatedFilePath) == CalculateMD5FromMemory(generatedString))
                    {
                        return;
                    }
                }

                File.WriteAllText(generatedFilePath, generatedString);

                GeneratedFilesCount++;
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
