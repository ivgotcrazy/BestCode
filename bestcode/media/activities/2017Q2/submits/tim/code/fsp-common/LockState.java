/**
 * 文件名：LockState
 * 版权：Copyright by www.fsmeeting.com
 * 描述：
 * 修改人：tangyh
 * 修改时间：2017/6/15
 * 修改内容：
 */

package com.fsmeeting.fsp.common.bean;

/**
 * 锁节点状态
 *
 * @author tangyh
 * @version 2017/6/15
 * @see LockState
 */
public class LockState {
    private boolean isLocked;

    private String lockPath;

    public boolean isLocked() {
        return isLocked;
    }

    public void setLocked(boolean locked) {
        isLocked = locked;
    }

    public String getLockPath() {
        return lockPath;
    }

    public void setLockPath(String lockPath) {
        this.lockPath = lockPath;
    }
}
