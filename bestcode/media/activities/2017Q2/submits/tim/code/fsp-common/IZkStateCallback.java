/**
 * 文件名：IZkStateCallback
 * 版权：Copyright by www.fsmeeting.com
 * 描述：
 * 修改人：tangyh
 * 修改时间：2017/6/8
 * 修改内容：
 */

package com.fsmeeting.fsp.common.zookeeper;

/**
 * zk连接状态改变时的回调类
 *
 * @author tangyh
 * @version 2017/6/8
 * @see IZkStateCallback
 */
public interface IZkStateCallback {
    void reConnected();

    void lostConnection();
}
