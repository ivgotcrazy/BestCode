/**
 * 文件名：ZkNodeServiceImpl
 * 版权：Copyright by www.fsmeeting.com
 * 描述：
 * 修改人：tangyh
 * 修改时间：2017/6/8
 * 修改内容：
 */

package com.fsmeeting.fsp.common.zookeeper.impl;

import com.fsmeeting.fsp.common.bean.NodeCache2Listener;
import com.fsmeeting.fsp.common.bean.PathChildrenCache2Listener;
import com.fsmeeting.fsp.common.util.Assert;
import com.fsmeeting.fsp.common.zookeeper.*;
import org.apache.curator.framework.CuratorFramework;
import org.apache.curator.framework.recipes.cache.NodeCache;
import org.apache.curator.framework.recipes.cache.NodeCacheListener;
import org.apache.curator.framework.recipes.cache.PathChildrenCache;
import org.apache.curator.framework.recipes.cache.PathChildrenCacheListener;
import org.apache.curator.framework.recipes.cache.ChildData;
import org.apache.zookeeper.CreateMode;
import org.apache.zookeeper.KeeperException;
import org.apache.zookeeper.data.ACL;
import org.apache.zookeeper.data.Stat;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.util.ArrayList;
import java.util.List;

/**
 * zk节点服务类
 *
 * @author tangyh
 * @version 2017/6/8
 * @see ZkNodeServiceImpl
 */
public class ZkNodeServiceImpl implements ZkNodeService {
    private Logger logger = LoggerFactory.getLogger(ZkNodeServiceImpl.class);

    private ZkConnection zkConnection;

    private ZkNodeServiceImpl() {
    }

    public ZkNodeServiceImpl(ZkConnection zkConnection) {
        this.zkConnection = zkConnection;
    }

    @Override
    public String createNode(String node, CreateMode mode, ArrayList<ACL> acls, String data) {
        Assert.notNull(node, "node path is null");
        Assert.notNull(mode, "CreateMode is null");

        String path = null;
        CuratorFramework client = zkConnection.getClient();
        if (client != null) {
            try {
                if (acls != null) {
                    path = client.create()
                            .creatingParentsIfNeeded()
                            .withMode(mode)
                            .withACL(acls)
                            .forPath(node, data.getBytes());
                } else {
                    path = client.create()
                            .creatingParentsIfNeeded()
                            .withMode(mode)
                            .forPath(node, data.getBytes());
                }
                logger.info("create node " + path + ",data:" + data);
                return path;
            } catch (KeeperException.NodeExistsException exception) {
                logger.error("Node Exists exception!node:" + node, exception);
                return com.fsmeeting.fsp.common.constant.Constant.ZK_NODE_EXIST + node;
            } catch (Exception exception) {
                logger.error("create node exception!node:" + node, exception);
            }
        }
        return null;
    }

    @Override
    public boolean delNode(String node) {
        Assert.notNull(node, "node is null");

        CuratorFramework client = zkConnection.getClient();
        if (client != null) {
            try {
                client.delete().deletingChildrenIfNeeded().withVersion(-1).forPath(node);
                logger.info("delete node " + node);
                return true;
            } catch (Exception exception) {
                logger.error("delete node " + node + " error!", exception);
            }
        }
        return false;
    }

    @Override
    public boolean checkNodeExist(String path) {
        Assert.notNull(path, "node path is null");

        CuratorFramework client = zkConnection.getClient();
        if (client != null) {
            try {
                Stat stat = client.checkExists().forPath(path);
                logger.info(path + " existed");
                return stat != null;
            } catch (Exception exception) {
                logger.error("checking node exist error!node:" + path, exception);
                return false;
            }
        }
        return false;
    }

    @Override
    public String getNodeData(String path) {
        Assert.notNull(path, "node path is null");

        CuratorFramework client = zkConnection.getClient();
        if (client != null) {
            Stat stat = new Stat();
            try {
                byte[] data = client.getData().storingStatIn(stat).forPath(path);
                return new String(data);
            } catch (Exception exception) {
                logger.error("get node data error!,node:" + path, exception);
                return null;
            }
        }
        return null;
    }

    @Override
    public List<String> getNodeChildren(String path) {
        Assert.notNull(path, "node path is null");

        CuratorFramework client = zkConnection.getClient();
        if (client != null) {
            try {
                List<String> nodes = client.getChildren().forPath(path);
                return nodes;
            } catch (Exception exception) {
                logger.error("get node children error!", exception);
            }
        }
        return null;
    }

    @Override
    public PathChildrenCache2Listener listenNodeChildren(String path, NodeChildrenListener listen) {
        Assert.notNull(path, "node path is null");

        CuratorFramework client = zkConnection.getClient();
        if (client != null) {
            final PathChildrenCache cache = new PathChildrenCache(client, path, true);
            try {
                cache.start();
                PathChildrenCache2Listener cache2Listener = new PathChildrenCache2Listener();
                PathChildrenCacheListener pathChildrenCacheListener = (client1, event) -> listen.listen(event);
                cache.getListenable().addListener(pathChildrenCacheListener);
                cache2Listener.setChildrenCache(cache);
                cache2Listener.setListener(pathChildrenCacheListener);
                return cache2Listener;
            } catch (Exception exception) {
                logger.error("PathChildrenCache start error!", exception);
            }
        }
        return null;
    }

    @Override
    public void updateNodeData(String path, String data) {
        Assert.notNull(path, "node path is null");
        Assert.notNull(data, "node path is null");

        CuratorFramework client = zkConnection.getClient();
        if (client != null) {
            try {
                client.setData().forPath(path, data.getBytes());
            } catch (Exception exception) {
                logger.error("update node error!", exception);
            }
        }
    }

    @Override
    public NodeCache2Listener listenNodeDel(String path, NodeDeleteCallback callback) {
        Assert.notNull(path, "node path is null");
        CuratorFramework client = zkConnection.getClient();

        if (client != null) {
            final NodeCache cache = new NodeCache(client, path);
            NodeCache2Listener nodeCache2Listener = new NodeCache2Listener();
            try {
                cache.start();
                NodeCacheListener nodeCacheListener = () -> {
                    ChildData childData = cache.getCurrentData();
                    if (childData == null) {
                        callback.callback();
                    }
                };
                cache.getListenable().addListener(nodeCacheListener);

                nodeCache2Listener.setCache(cache);
                nodeCache2Listener.setNodeCacheListener(nodeCacheListener);
            } catch (Exception exception) {
                logger.error("handle node error!", exception);
            }
            return nodeCache2Listener;
        }
        return null;
    }
}
