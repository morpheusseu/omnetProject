# omnetProject

一个几乎全新的网络模型诞生了

多负载的LWSN，负载数（或RD）通过节点数自动推算，您只需要修改符合公式的节点数量就可以得到全新的网络拓扑

之前的网络拓扑可以看作RD=1的特殊情况

      删除了多种消息（如INFO、TURNON、TURNOFF），替代为UPDATE

      修改了一些原本通过INFO消息传送的参数，放入NED文件中

      将flag放入NED文件中，从而取代了TURNON、TURNOFF消息
      
      将load（即控制RD的参数），numOfNodes（控制节点数量的参数）放入NED文件的network之下，改善了易用性

其他特性敬请自行探索，如有各种代码错误或漏洞请及时反馈，谢谢合作
