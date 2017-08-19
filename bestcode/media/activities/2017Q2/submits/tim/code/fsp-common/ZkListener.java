/**
 * 文件名：ZkListener
 * 版权：Copyright by www.fsmeeting.com
 * 描述：
 * 修改人：kezs
 * 修改时间：2017/6/1
 * 修改内容：
 */

package com.fsmeeting.fsp.common.zookeeper;

import org.apache.curator.framework.CuratorFramework;

import java.util.Arrays;
import java.util.List;


/**
 * 用来监听zookeeper 连接以及节点变化
 *
 * @author kezs tangyh
 * @version 2017/6/1
 * @see ZkListener
 */
public class ZkListener {
    private ZkConnection zkConnection;

    private static IZkStateCallback[] callbacks;

    private ZkListener() {
    }

    public ZkListener(ZkConnection zkConnection, IZkStateCallback... callbacks) {
        this.zkConnection = zkConnection;
        this.callbacks = Arrays.asList(callbacks).toArray(new IZkStateCallback[callbacks.length]);
    }

    public ZkListener(ZkConnection zkConnection) {
        this.zkConnection = zkConnection;
    }

    /**
     * 添加zk重连后的回调函数
     */
    public synchronized void addCallback(IZkStateCallback... callbacks1) {
        if (callbacks == null) {
            callbacks = Arrays.asList(callbacks1).toArray(new IZkStateCallback[callbacks1.length]);
        } else {
            int size1 = callbacks.length;
            int size2 = callbacks1.length;
            IZkStateCallback[] callbacks2 = new IZkStateCallback[size1 + size2];
            System.arraycopy(callbacks, 0, callbacks2, 0, size1);
            System.arraycopy(callbacks1, 0, callbacks2, size1, size2);
            callbacks = callbacks2;
        }
    }

	/**
     * 删除回调函数
     */
    public synchronized void removeCallback(IZkStateCallback callbacks1) {
        if (callbacks != null && callbacks.length > 0) {
            List<IZkStateCallback> list = Arrays.asList(callbacks);
            list.remove(callbacks1);
            callbacks = list.toArray(new IZkStateCallback[list.size()]);
        }
    }

    /**
     * 监听连接以及节点变化
     */
    public void listen() {
        CuratorFramework client = zkConnection.getClient();
        client.getConnectionStateListenable().addListener((curatorFramework, connectionState) -> {
            if (connectionState.isConnected()) {
                for (int i = 0; i < callbacks.length; i++) {
                    callbacks[i].reConnected();
                }
            } else {
                for (int i = 0; i < callbacks.length; i++) {
                    callbacks[i].lostConnection();
                }
            }
        });
    }
}
