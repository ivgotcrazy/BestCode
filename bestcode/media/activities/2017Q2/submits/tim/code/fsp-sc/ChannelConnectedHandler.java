/**
 * 文件名：ChannelConnectedHandler
 * 版权：Copyright by www.fsmeeting.com
 * 描述：
 * 修改人：kezs
 * 修改时间：2017/2/22
 * 修改内容：
 */

package com.fsmeeting.fsp.sc.handler;

import com.fsmeeting.fsp.common.handler.IProtobuf;
import com.fsmeeting.fsp.common.lock.Lock;
import com.fsmeeting.fsp.common.util.NumberUtil;
import com.fsmeeting.fsp.proto.common.CommonResponse;
import com.fsmeeting.fsp.proto.common.DataDirection;
import com.fsmeeting.fsp.proto.common.ResponseCode;
import com.fsmeeting.fsp.proto.sc.ChannelConnected;
import com.fsmeeting.fsp.proto.sc.ChannelConnectedRsp;
import com.fsmeeting.fsp.proto.sc.ProtoDictionary;
import com.fsmeeting.fsp.sc.common.Common;
import com.fsmeeting.fsp.sc.service.CommonService;
import com.fsmeeting.fsp.sc.service.MessageResponseService;
import com.fsmeeting.fsp.sc.service.RequestCacheService;
import com.fsmeeting.fsp.sc.service.RouterService;
import com.google.protobuf.InvalidProtocolBufferException;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

/**
 * 处理SS通知SC已经和NC建立通道连接
 *
 * @author kezs tangyh
 * @version 2017/2/22
 * @see ChannelConnectedHandler
 */
@Service("channelConnectedHandler")
public class ChannelConnectedHandler extends AbstractScHandler implements IProtobuf<ChannelConnected> {

    @Autowired
    private CommonService commonService;

    @Autowired
    private RouterService routerService;

    @Autowired
    private MessageResponseService responseService;

    @Autowired
    private RequestCacheService requestCacheService;

    @Autowired
    private Lock lock;

    @Override
    public void handle() {
        ChannelConnected channelConnected = deserialize(messageData);
        DataDirection direction = channelConnected.getDirection();
        String streamId = channelConnected.getStreamId();
        String serviceInstanceId = channelConnected.getServiceInstanceId();
        String clientId = channelConnected.getClientId();

        String rspMsg = "Saving channel connect info failed";
        byte[] result;
        boolean flag = saveChannelConnected(clientId, serviceInstanceId, streamId, direction);
        if (flag) {
            //流的发布客户端登录源流服务器（nc发布端连接ss的时候）
            if (direction.getNumber() == DataDirection.Sending_VALUE) {
                logger.info("数据方向为发送，释放锁");
                String rootPath = Common.getGetSuperRootLockPath();
                String lockRootPath = rootPath + streamId;
                lock.unLock(lockRootPath);
            }
            rspMsg = clientId + " connected " + serviceInstanceId + ",streamId:" + streamId;
            result = builtResponseResult(ResponseCode.EnumSuccess, rspMsg);
        } else {
            result = builtResponseResult(ResponseCode.EnumError, rspMsg);
        }
        responseService.response(messageSequence, messageNumber, msgTopic, result);
    }

    /**
     * Description: 保存通道连接信息
     *
     * @param clientId          客户端id
     * @param serviceInstanceId 服务实例
     * @param streamId          流id
     * @param direction         流方向
     * @return 返回是否保存成功
     */
    private boolean saveChannelConnected(String clientId, String serviceInstanceId, String streamId,
                                         DataDirection direction) {
        fsp.sss.DataDirection dataDirection = (direction == DataDirection.Receiving
                ? fsp.sss.DataDirection.Recv : fsp.sss.DataDirection.Send);

        if (clientId.substring(0, 2).equals("0;")) {
            clientId = clientId.substring(2);
        }
        return iceProxy.saveChannelConnect(clientId, serviceInstanceId, streamId, dataDirection);
    }

    /**
     * Description: 构建处理返回结果
     *
     * @param code:返回状态，msg：返回信息
     */
    private byte[] builtResponseResult(ResponseCode code, String msg) {
        ChannelConnectedRsp.Builder rsp = ChannelConnectedRsp.newBuilder();
        CommonResponse response = responseService.buildCommonResponse(code, msg);
        rsp.setResponse(response);
        return serialize(rsp.build());
    }

    @Override
    public ChannelConnected deserialize(byte[] protoData) {
        try {
            return ChannelConnected.parseFrom(protoData);
        } catch (InvalidProtocolBufferException ex) {
            logger.error("deserialize error", ex);
        }
        return null;
    }

    @Override
    protected Integer getHandlerType() {
        return ProtoDictionary.Enum2ChannelConnected_VALUE;
    }

    @Override
    public byte[] getMessageNumber() {
        return NumberUtil.int2byte(ProtoDictionary.Enum2ChannelConnectedRsp_VALUE);
    }
}
