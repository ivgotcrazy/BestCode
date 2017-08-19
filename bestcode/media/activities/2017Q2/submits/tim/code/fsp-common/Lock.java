/**
 * 文件名：LockState
 * 版权：Copyright by www.fsmeeting.com
 * 描述：
 * 修改人：tangyh
 * 修改时间：2017/6/12
 * 修改内容：
 */

package com.fsmeeting.fsp.common.lock;

import com.fsmeeting.fsp.common.bean.LockState;
import com.fsmeeting.fsp.common.bean.NodeCache2Listener;
import com.fsmeeting.fsp.common.util.Assert;
import com.fsmeeting.fsp.common.zookeeper.NodeDeleteCallback;
import com.fsmeeting.fsp.common.zookeeper.impl.ZkNodeServiceImpl;
import org.apache.curator.framework.recipes.cache.NodeCache;
import org.apache.curator.framework.recipes.cache.NodeCacheListener;
import org.apache.zookeeper.CreateMode;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;

/**
 * 分布式锁
 *
 * @author tangyh
 * @version 2017/6/12
 * @see Lock
 */
public class Lock {
    private Logger logger = LoggerFactory.getLogger(Lock.class);

    private ZkNodeServiceImpl zkNodeService;

    private Lock() {
    }

    public Lock(ZkNodeServiceImpl zkNodeService) {
        this.zkNodeService = zkNodeService;
    }

    /**
    * 获得锁
    */
    public LockState tryLock(String lockRootPath, String node, int timout) {
        Assert.notNull(lockRootPath, "lock root path is null");
        Assert.notNull(node, "node is null");

        LockState state = new LockState();
        state.setLocked(false);

        String nodePath = lockRootPath + "/" + node;
        String path = zkNodeService.createNode(nodePath, CreateMode.EPHEMERAL_SEQUENTIAL, null, nodePath);
        List<String> nodes = zkNodeService.getNodeChildren(lockRootPath);
        int nodeSize = node.length();
        int pos = path.lastIndexOf(node) + nodeSize;
        String node1 = path.substring(pos);
        int nodeNumber = Integer.parseInt(node1);
        List<Integer> numbers = new ArrayList<>();
        nodes.forEach((node2) -> {
            int number = Integer.parseInt(node2.substring(nodeSize));
            numbers.add(number);
        });
        if (numbers.size() > 0) {
            numbers.sort((num1, num2) -> num1 - num2);
            int min = numbers.get(0);
            if (nodeNumber == min) {
                state.setLocked(true);
                Thread thread = new Thread(() -> lockTimeout(path, timout));      //锁的超时处理
                thread.start();
            } else {
                zkNodeService.delNode(path);
            }
            state.setLockPath(nodePath + min);
        }
        return state;
    }

    public boolean isLocked(String path) {
        return zkNodeService.checkNodeExist(path);
    }

    /**
    * 清除锁节点监听对象
    */
    public void cleanNodeCacheListener(String key, Map<String, NodeCache2Listener> cache2listenerMap) {
        if (key != null) {
            NodeCache2Listener nodeCache2Listener = cache2listenerMap.get(key);
            if (nodeCache2Listener != null) {
                NodeCache cache = nodeCache2Listener.getCache();
                NodeCacheListener listener = nodeCache2Listener.getNodeCacheListener();
                if (listener != null) {
                    cache.getListenable().removeListener(listener);
                }
                try {
                    cache.close();
                } catch (IOException exception) {
                    logger.error("node cache failed to close", exception);
                }
            }
        }
    }

    /**
    * 锁的超时处理
    */
    private synchronized void lockTimeout(String lockPath, int timeout) {
        logger.info("lock timeout:" + timeout);
        long totalTime = 1000 * timeout;
        while (true) {
            try {
                wait(totalTime);
                break;
            } catch (InterruptedException exception) {
                logger.error("lock timeout task error!lock path:" + lockPath, exception);
            }
        }
        boolean flag = zkNodeService.checkNodeExist(lockPath);
        if (flag) {
            logger.error("lock node timeout!!!!!!!!!!!!");
            unLock(lockPath);
        }
    }

    /**
    * 释放锁
    */
    public void unLock(String lockPath) {
        Assert.notNull(lockPath, "lock path is null");

        boolean flag = zkNodeService.checkNodeExist(lockPath);
        if (flag) {
            zkNodeService.delNode(lockPath);
        }
    }

    /**
    * 监听锁
    */
    public NodeCache2Listener listenerLock(String path, NodeDeleteCallback callback) {
        return zkNodeService.listenNodeDel(path, new MyNodeDelHandlerImpl(callback));
    }

    /**
    * 子节点监听类
    */
    private class MyNodeDelHandlerImpl implements NodeDeleteCallback {
        private NodeDeleteCallback callback;

        public MyNodeDelHandlerImpl(NodeDeleteCallback callback) {
            this.callback = callback;
        }

        @Override
        public void callback() {
            callback.callback();
        }
    }
}
