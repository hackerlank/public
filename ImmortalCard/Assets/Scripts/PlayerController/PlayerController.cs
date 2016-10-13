using UnityEngine;
using System.Collections;
using Google.Protobuf;

public interface PlayerController {
	void onMessage(Player player,IMessage msg);
}
