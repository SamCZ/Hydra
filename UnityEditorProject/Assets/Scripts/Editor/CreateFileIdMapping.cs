using System.Collections;
using System.Collections.Generic;
using System.IO;
using UnityEditor;
using UnityEngine;

public class FileMapping
{
    public string FilePath;
    public string GUID;
    public Dictionary<string, long> FileIdMap;
}

public class CreateFileIdMapping
{
    
    [MenuItem("Hydra/Map files")]
    public static void CreateMap()
    {
        List<FileMapping> fileMap = new List<FileMapping>();

        string sAssetFolderPath = "Assets";
        string[] aFilePaths = Directory.GetFiles(sAssetFolderPath, "*", SearchOption.AllDirectories);
        
        foreach (string sFilePath in aFilePaths)
        {
            if (Path.GetExtension(sFilePath) == ".FBX" || Path.GetExtension(sFilePath) == ".fbx")
            {
                GameObject objAsset = AssetDatabase.LoadAssetAtPath<GameObject>(sFilePath);

                FileMapping mapping = new FileMapping();
                mapping.FilePath = sFilePath.Replace('\\', '/');
                mapping.FileIdMap = new Dictionary<string, long>();
                
                for (int i = 0; i < objAsset.transform.childCount; i++)
                {
                    GameObject child = objAsset.transform.GetChild(i).gameObject;

                    MeshFilter filter = child.GetComponent<MeshFilter>();

                    if(filter)
                    {
                        string guid;
                        long localId;
                        AssetDatabase.TryGetGUIDAndLocalFileIdentifier(filter.sharedMesh, out guid, out localId);

                        mapping.GUID = guid;

                        mapping.FileIdMap.Add(child.name, localId);
                    }
                }
                
                if(mapping.FileIdMap.Count > 0)
                {
                    fileMap.Add(mapping);
                }
            }
        }

        List<string> lines = new List<string>();

        string NL = "\r\n";

        foreach(FileMapping map in fileMap)
        {
            string line = map.FilePath + ":" + NL;

            line += "  GUID: " + map.GUID + NL;

            int i = 0;
            foreach(KeyValuePair<string, long> pair in map.FileIdMap)
            {
                i++;

                line += "  " + pair.Key + ": " + pair.Value.ToString();

                if(i != map.FileIdMap.Count)
                {
                    line += NL;
                }
            }


            lines.Add(line);
        }

        File.WriteAllLines(sAssetFolderPath + "/FileAssetMap.yml", lines);

        Debug.Log("Mapping done.");
    }

}
