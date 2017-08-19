/**
 * 文件名：NodeDeleteCallback
 * 版权：Copyright by www.fsmeeting.com
 * 描述：
 * 修改人：tangyh
 * 修改时间：2017/6/15
 * 修改内容：
 */

package com.fsmeeting.fsp.common.zookeeper;

/**
 * 节点删除回调接口
 *
 * @author tangyh
 * @version 2017/6/15
 * @see NodeDeleteCallback
 */
public interface NodeDeleteCallback {

    void callback();
}
