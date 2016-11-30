using UnityEngine;
using System.Collections;

public class CSV {
	Hashtable _table=new Hashtable();

	public CSV(string buf){
		if(buf!=null){
			string[] lines=buf.Split('\n');
			if(lines.Length>1){
				//head
				string[] head=Utils.SplitTsvLine(lines[0]);
				if(head.Length>1){
					//data
					for(int i=1;i<lines.Length;++i){
						string[] fields=Utils.SplitTsvLine(lines[i]);
						if(fields.Length>0){
							//key
							string key=fields[0];
							if(key=="")continue;
							//value
							Hashtable val=new Hashtable();
							for(int j=1;j<fields.Length;++j){
								val[head[j]]=fields[j];
							}
							_table[key]=val;
						}
					}
				}
			}
			//_print ();
		}
	}

	public string value(string key,string field){
		string v=null;
		if(_table.ContainsKey(key)){
			Hashtable fields=_table[key] as Hashtable;
			if(fields!=null&&fields.ContainsKey(field))
				v=(string)fields[field];
		}
		return v;
	}

	void _print(){
		Debug.Log("localization csv loaded, "+_table.Count+" lines");
		foreach(DictionaryEntry i in _table){
			Hashtable fields=i.Value as Hashtable;
			if(fields!=null)foreach(DictionaryEntry j in fields)
				Debug.Log(i.Key+"( "+j.Key+" )"+"="+j.Value);
		}
	}
}
