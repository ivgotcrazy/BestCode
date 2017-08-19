/**
 * 文件名：GetSuperiorStreamServerHandler
 * 版权：Copyright by www.fsmeeting.com
 * 描述：
 * 修改人：kezs
 * 修改时间：2017/2/22
 * 修改内容：
 */

package com.fsmeeting.fsp.sc.handler;

import com.fsmeeting.fsp.common.bean.LockState;
import com.fsmeeting.fsp.common.bean.NodeCache2Listener;
import com.fsmeeting.fsp.common.handler.IProtobuf;
import com.fsmeeting.fsp.common.lock.Lock;
import com.fsmeeting.fsp.common.util.NumberUtil;
import com.fsmeeting.fsp.common.zookeeper.IZkStateCallback;
import com.fsmeeting.fsp.common.zookeeper.NodeDeleteCallback;
import com.fsmeeting.fsp.common.zookeeper.ZkListener;
import com.fsmeeting.fsp.proto.common.CommonResponse;
import com.fsmeeting.fsp.proto.common.ResponseCode;
import com.fsmeeting.fsp.proto.common.StreamServer;
import com.fsmeeting.fsp.proto.cp.SelectStreamServer;
import com.fsmeeting.fsp.proto.sc.GetSuperiorStreamServer;
import com.fsmeeting.fsp.proto.sc.GetSuperiorStreamServerRsp;
import com.fsmeeting.fsp.proto.sc.ProtoDictionary;
import com.fsmeeting.fsp.sc.common.Common;
import com.fsmeeting.fsp.sc.entry.RequestInfo;
import com.fsmeeting.fsp.sc.request.SelectStreamServerReq;
import com.fsmeeting.fsp.sc.service.CommonService;
import com.fsmeeting.fsp.sc.service.MessageResponseService;
import com.fsmeeting.fsp.sc.service.RequestCacheService;
import com.fsmeeting.fsp.sc.service.RouterService;
import com.google.protobuf.InvalidProtocolBufferException;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

/**
 * 处理SS或者GS向SC请求上级SS节点的逻辑实现
 *
 * @author kezs tangyh
 * @version 2017/2/22
 * @see GetSuperiorStreamServerHandler
 */
@Service("getSuperiorStreamServerHandler")
public class GetSuperiorStreamServerHandler extends AbstractScHandler implements IProtobuf<GetSuperiorStreamServer> {

    @Autowired
    private SelectStreamServerReq selectStreamServerReq;

    @Autowired
    private RouterService routerService;

    @Autowired
    private CommonService commonService;

    @Autowired
    private MessageResponseService responseService;

    @Autowired
    private RequestCacheService requestCacheService;

    @Autowired
    private Lock lock;

    @Autowired
    private ZkListener zkListener;

    /**
     * 缓存流id对应的锁节点监听对象
     * */
    private static final Map<String, NodeCache2Listener> cache2listenerMap = new ConcurrentHashMap<>();

    /**
     * 缓存流id对应的zk重连回调函数
     * */
    private static final Map<String, IZkStateCallback> streamId2ZkStateCallbackMap = new ConcurrentHashMap<>();

    /**
     * 缓存流id对应的请求
     * */
    private static final Map<String, List<RequestInfo>> streamId2Requests = new HashMap<>();

    @Override
    public void handle() {
        GetSuperiorStreamServer getSuperiorStreamServer = deserialize(messageData);
        String streamId = getSuperiorStreamServer.getStreamId();
        String serverInstanceId = getSuperiorStreamServer.getServiceInstanceId();

		//锁的根目录：/sclock/streamId
        String lockRootPath = Common.getGetSuperRootLockPath() + streamId;
        int timeout = Common.getGetSuperLockTimeout();
        LockState state = lock.tryLock(lockRootPath, streamId, timeout);
        String lockPath = state.getLockPath();
        //获得锁
        if (state.isLocked()) {
            logger.info("===========================获得锁=================");
            String superiorStreamServer = commonService.getSuperiorStreamServer(streamId, serverInstanceId);
            //如果没有上一级节点
            if (superiorStreamServer == null) {
                logger.info(serverInstanceId + "没有上一级节点,接下来判断流的发布端是否已经选择流服务器");
                String sourceInstance = commonService.getSourceInstance(streamId); 
                logger.info("GetSuperiorStreamServerHandler,streamId:" + streamId + " serverInstanceId:"
                        + serverInstanceId + " sourceInstance:" + sourceInstance);
                //如果流的源节点没有选择
                if (sourceInstance == null) {
                    logger.info("No source instance，and no one request to select source server,"
                            + "request publisher to select source server.");
                    //请求流的发布者选择源服务器
                    requestSelectSource(streamId);
                } else {
                    superiorStreamServer = routerService.voteOptimalStreamServer(streamId, sourceInstance,
                            serverInstanceId);
                }
            }
            //如果有上一级节点
            if (superiorStreamServer != null && !superiorStreamServer.isEmpty()) {
                logger.info("上一级节点不为空，返回获得上一级节点请求");
                byte[] reqData;
                //todo 异常处理
                StreamServer streamServer = commonService.getStreamServerByInstanceId(superiorStreamServer);
                reqData = this.buildSuperiorStreamServerRsp(ResponseCode.EnumSuccess, streamServer,
                        "选择" + superiorStreamServer + "作为" + serverInstanceId + "的上一级节点");
                logger.info("选择" + superiorStreamServer + "作为" + serverInstanceId + "的上一级节点");
                responseService.response(messageSequence, messageNumber, msgTopic, reqData);
                lock.unLock(lockPath);
                return;
            }
        }
        //保存获取上一级节点请求
        saveGetSuperiorStreamRequest(streamId, messageSequence, msgTopic, serverInstanceId);

        NodeCache2Listener nodeCache2Listener = cache2listenerMap.get(streamId);
        if (nodeCache2Listener == null) {
            listenLock(streamId, lockPath);
        }
        listenZkReconnect(streamId);
    }

    /**
     * Description:缓存请求到本地
     *
     * @param streamId   流id
     * @param sequence   请求序号
     * @param topic      topic
     * @param instanceId 服务实例
     */
    public void saveGetSuperiorStreamRequest(String streamId, byte[] sequence, String topic, String instanceId) {
        List<RequestInfo> requestInfos = streamId2Requests.get(streamId);
        if (requestInfos == null) {
            requestInfos = new ArrayList<>();
        }

        RequestInfo requestInfo = new RequestInfo();
        requestInfo.setSequence(sequence);
        requestInfo.setTopic(topic);
        requestInfo.setInstanceId(instanceId);

        requestInfos.add(requestInfo);
        streamId2Requests.put(streamId, requestInfos);
    }

	/**
     * 请求客户端选择源节点
     */
    private synchronized void requestSelectSource(String streamId) {
        SelectStreamServer streamServer = commonService.selectSourceServer(streamId);
        String clientId = commonService.getSourceClient(streamId);
        String cpServerInstance = commonService.getCPInstance(clientId);
        byte[] reqData = serialize(streamServer);
        selectStreamServerReq.resquest(cpServerInstance, reqData);
        logger.info("save get superior stream request.");
    }

    /**
     * 回复获得上一级服务器请求
     */
    private synchronized void rspGetSuperiorStreamServer(String streamId, String serviceInstanceId) {
        logger.info("======================回复获取上一级节点请求=====================================");
        List<RequestInfo> requestInfos = streamId2Requests.get(streamId);
        if (requestInfos != null) {
            for (RequestInfo requestInfo : requestInfos) {
                String destInstance = requestInfo.getInstanceId();
                String superiorStreamServer = routerService.voteOptimalStreamServer(streamId, serviceInstanceId,
                        destInstance);
                byte[] reqData = null;
                if (superiorStreamServer != null && !superiorStreamServer.isEmpty()) {
                    //todo 服务器类型不确定
                    StreamServer streamServer = commonService.getStreamServerByInstanceId(superiorStreamServer);
                    reqData = this.buildSuperiorStreamServerRsp(ResponseCode.EnumSuccess, streamServer,
                            "选择" + superiorStreamServer + "作为" + destInstance + "的上一级节点");
                    byte[] sequence = requestInfo.getSequence();
                    String topic = requestInfo.getTopic();
                    responseService.response(sequence, messageNumber, topic, reqData);
                }
            }
            streamId2Requests.remove(streamId);
        }
        IZkStateCallback zkStateCallback = streamId2ZkStateCallbackMap.get(streamId);
        if (zkStateCallback != null) {
            streamId2ZkStateCallbackMap.remove(streamId);
            zkListener.removeCallback(zkStateCallback);
        }
    }

    /**
     * Description: 构建GetSuperiorStreamServerRsp对象
     */
    private byte[] buildSuperiorStreamServerRsp(ResponseCode responseCode, StreamServer streamServer, String msg) {
        CommonResponse.Builder commonResponse = CommonResponse.newBuilder();
        GetSuperiorStreamServerRsp.Builder superiorStreamServer = GetSuperiorStreamServerRsp.newBuilder();
        if (responseCode.equals(ResponseCode.EnumSuccess)) {
            superiorStreamServer.setStreamServer(streamServer);
        }
        commonResponse.setResponseMsg(msg);
        commonResponse.setResponseCode(responseCode);
        superiorStreamServer.setResponse(commonResponse);
        return serialize(superiorStreamServer.build());
    }

    @Override
    public GetSuperiorStreamServer deserialize(byte[] protoData) {
        try {
            return GetSuperiorStreamServer.parseFrom(protoData);
        } catch (InvalidProtocolBufferException ex) {
            logger.error(ex.getMessage(), ex);
        }
        return null;
    }

    @Override
    protected Integer getHandlerType() {
        return ProtoDictionary.Enum2GetSuperiorStreamServer_VALUE;
    }

    @Override
    public byte[] getMessageNumber() {
        return NumberUtil.int2byte(ProtoDictionary.Enum2GetSuperiorStreamServerRsp_VALUE);
    }

    /**
     * 监听锁节点的删除
     */
    private synchronized void listenLock(String streamId, String lockPath) {
        if (lockPath != null) {
            logger.info("监听目录：" + lockPath);
            LockDeleteEvent lockDeleteEvent = new LockDeleteEvent(streamId);
            NodeCache2Listener nodeCache2Listener = lock.listenerLock(lockPath, lockDeleteEvent);
            cache2listenerMap.put(streamId, nodeCache2Listener);
        }
    }

    /**
     * 监听zookeeper的重连
     */
    private void listenZkReconnect(String streamId) {
        IZkStateCallback zkStateCallback = streamId2ZkStateCallbackMap.get(streamId);
        if (zkStateCallback == null) {
            zkStateCallback = new LockNodeReConnect(streamId);
            IZkStateCallback[] callbacks = new IZkStateCallback[]{zkStateCallback};
            zkListener.addCallback(callbacks);
            streamId2ZkStateCallbackMap.put(streamId, zkStateCallback);
        }
    }

    private synchronized void lockDelAction(String streamId) {
        String sourceInstance = commonService.getSourceInstance(streamId);
        String rootPath = Common.getGetSuperRootLockPath();
        String lockRootPath = rootPath + streamId;

        //流的源节点为空，则重新获得锁
        if (sourceInstance == null) {
            logger.info("源服务器为空。");
            int timout = Common.getGetSuperLockTimeout();
            LockState state = lock.tryLock(lockRootPath, streamId, timout);
            String lockPath = state.getLockPath();
            //竞争到锁
            if (state.isLocked()) {
                requestSelectSource(streamId);
            }
            listenLock(streamId, lockPath);
            listenZkReconnect(streamId);
        } else {
            rspGetSuperiorStreamServer(streamId, sourceInstance);
        }
    }

    /**
     * 监听zookeeper的重连事件,如果zk断开连接，那么之前监听的
     */
    private class LockNodeReConnect implements IZkStateCallback {

        private String streamId;

        LockNodeReConnect(String streamId) {
            this.streamId = streamId;
        }

        @Override
        public void reConnected() {
            lockDelAction(streamId);
        }

        @Override
        public void lostConnection() {
            lock.cleanNodeCacheListener(streamId, cache2listenerMap);
        }
    }

    /**
     * 监听锁节点的删除事件
     */
    private class LockDeleteEvent implements NodeDeleteCallback {
        private String streamId;

        protected LockDeleteEvent(String streamId) {
            this.streamId = streamId;
        }

        @Override
        public void callback() {
            logger.info("锁节点被删除。。。。。。。。。。。。。。。。。。");
            lock.cleanNodeCacheListener(streamId, cache2listenerMap);
            lockDelAction(streamId);
        }
    }
}
