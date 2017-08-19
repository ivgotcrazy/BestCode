/**
 * 文件名：ZkNodeService
 * 版权：Copyright by www.fsmeeting.com
 * 描述：
 * 修改人：tangyh
 * 修改时间：2017/5/9
 * 修改内容：
 */

package com.fsmeeting.fsp.common.zookeeper;

import com.fsmeeting.fsp.common.bean.NodeCache2Listener;
import com.fsmeeting.fsp.common.bean.PathChildrenCache2Listener;
import org.apache.zookeeper.CreateMode;
import org.apache.zookeeper.data.ACL;

import java.util.ArrayList;
import java.util.List;

/**
 * 对zk节点操作的接口
 *
 * @author tangyh
 * @version 2017/5/9
 * @see ZkNodeService
 */
public interface ZkNodeService {

    String createNode(String node, CreateMode mode, ArrayList<ACL> acls, String data);

    boolean delNode(String node);

    boolean checkNodeExist(String path);

    String getNodeData(String path);

    List<String> getNodeChildren(String path);

    PathChildrenCache2Listener listenNodeChildren(String path, NodeChildrenListener callback);

    void updateNodeData(String path, String data);

    NodeCache2Listener listenNodeDel(String path, NodeDeleteCallback callback);
}
