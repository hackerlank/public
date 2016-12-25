using UnityEngine;
using UnityEngine.UI;
using System.Collections;
using Proto3;

public class RuleIcon : MonoBehaviour {

	public Image		Icon;
	public Toggle		toggle;
	public ToggleGroup	Group;

	static public ToggleGroup	RuleGroup=null;

	pb_enum	category;
	public pb_enum Value{
		set{
			category=value;
			if(RuleSprites.Instance!=null && 
			   RuleSprites.Instance.dict.ContainsKey(category)){
				
				var sprite=RuleSprites.Instance.dict[category];
				Icon.sprite=sprite;
			}

			if(RuleGroup==null)
				RuleGroup=Group;
			else{
				toggle.group=RuleGroup;
			}
		}
		get{
			return category;
		}
	}
	public void OnGame(){
		if(toggle.isOn)
			EnterPanel.Instance.GameCategory=this;
	}
	
	public static string rule2name(pb_enum rule){
		switch(rule){
			//phz
		case pb_enum.PhzSy: return "邵阳字牌";
		case pb_enum.PhzSybp: return "邵阳剥皮";
		case pb_enum.PhzLd: return "娄底放炮";
		case pb_enum.PhzHh: return "怀化红拐弯";
		case pb_enum.PhzCdQmt: return "常德全名堂";
		case pb_enum.PhzCdHhd: return "常德红黑点";
		case pb_enum.PhzCs: return "长沙";
		case pb_enum.PhzXxGhz: return "湘乡告胡子";
		case pb_enum.PhzHy: return "衡阳六条枪";
		case pb_enum.PhzYzSbw: return "永州双霸王";
		case pb_enum.PhzPeghz: return "碰胡子";
		case pb_enum.PhzScEqs: return "四川二七十";
		case pb_enum.PhzCz: return "郴州跑胡子";
		case pb_enum.PhzGx: return "广西跑胡子";
			//mahjong
		case pb_enum.MjSichuan: return "四川麻将";
		case pb_enum.MjGuangdong: return "广东麻将";
		case pb_enum.MjHunan: return "湖南麻将";
		case pb_enum.MjFujian: return "福建麻将";
		case pb_enum.MjZhejiang: return "浙江麻将";
			//ddz
		case pb_enum.DdzClasic: return "经典斗地主";
		case pb_enum.DdzFor4: return "四人斗地主";
		default:
			return "???";
		}
	}
}
