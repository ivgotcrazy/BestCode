/*
 * 文件名：NoticeController.java
 * 版权：Copyright by www.fsmeeting.com
 * 描述：
 * 修改人：pich
 * 修改时间：2017年5月12日
 * 修改内容：
 */

package com.fs.party.article.controller.api;

import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import org.apache.log4j.Logger;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.cloud.client.ServiceInstance;
import org.springframework.cloud.client.discovery.DiscoveryClient;
import org.springframework.ui.ModelMap;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestMethod;
import org.springframework.web.bind.annotation.ResponseBody;
import org.springframework.web.bind.annotation.RestController;

import com.fs.party.article.common.ArticleStatusEnum;
import com.fs.party.article.exception.BannerExceptin;
import com.fs.party.article.exception.NoticeException;
import com.fs.party.article.pojo.Notice;
import com.fs.party.article.pojo.vo.NoticeInfo;
import com.fs.party.article.service.INoticeService;
import com.fs.party.depeds.annotation.OperationLog;
import com.fs.party.depeds.base.controller.BaseAbstractController;
import com.fs.party.depeds.base.exception.FSException;
import com.fs.party.depeds.constant.CommonConstants;
import com.fs.party.depeds.utils.Tools;

@RestController
@RequestMapping(value = "/notice")
@ResponseBody
public class NoticeController extends BaseAbstractController {

    private final Logger logger = Logger.getLogger(getClass());

    @Autowired
    private DiscoveryClient client;
    
    @Autowired
    private INoticeService noticeService;

    /**.
     *
     */
    @RequestMapping(value = "/addNotice", method = RequestMethod.POST)
    @OperationLog("添加新闻公告")
    @ResponseBody
    public ModelMap addNotice(@RequestBody Notice notice) {
        ModelMap resultMap = new ModelMap();
        try {
            if (Tools.isEmpty(notice.getTitle())) {
                throw new FSException(NoticeException.TITLE_IS_NULL, "标题不能为空");
            } else if (Tools.isEmpty(notice.getContent())) {
                throw new FSException(NoticeException.CONTENT_IS_NULL, "内容不能为空");
            } else if (Tools.isEmpty(notice.getType())) {
                throw new FSException(NoticeException.TYPE_IS_NULL, "类型不能为空");
            } else if (Tools.isEmpty(notice.getCreateUserId())) {
                throw new FSException(NoticeException.CREATEUSERID_IS_NULL, "创建人ID不能为空");
            } else if (Tools.isEmpty(notice.getOrgId())) {
                throw new FSException(NoticeException.ORGID_IS_NULL, "所属组织不能为空");
            }
            noticeService.addNotice(notice);
            resultMap.put(CommonConstants.RESULT_STATUS, CommonConstants.SUCCESS_STATUS);
        } catch (FSException exc) {
            // 请保持所有代码一直性，没有特殊处理请使用该代码.
            this.printExction(resultMap, exc);
        }
        this.printLog(resultMap);
        return resultMap;
    }

    /**.
     *
     */
    @RequestMapping(value = "/delNotice", method = RequestMethod.POST)
    @OperationLog("删除新闻公告")
    @ResponseBody
    public ModelMap delNotice(@RequestBody NoticeInfo notice) {
        ModelMap resultMap = new ModelMap();
        try {
            if (Tools.isEmpty(notice.getIds())) {
                throw new FSException(NoticeException.DELID_IS_NULL, "删除ID值不能为空");
            }
            noticeService.delNotice(notice.getIds());
            resultMap.put(CommonConstants.RESULT_STATUS, CommonConstants.SUCCESS_STATUS);
        } catch (FSException exc) {
            // 请保持所有代码一直性，没有特殊处理请使用该代码.
            this.printExction(resultMap, exc);
        }
        this.printLog(resultMap);
        return resultMap;
    }

    /**.
     *
     */
    @RequestMapping(value = "/modifyNotice", method = RequestMethod.POST)
    @OperationLog("修改新闻公告")
    @ResponseBody
    public ModelMap modifyNotice(@RequestBody Notice notice) {
        ModelMap resultMap = new ModelMap();
        try {
            if (Tools.isEmpty(notice.getId())) {
                throw new FSException(NoticeException.ID_IS_NULL, "修改对应ID不能为空");
            }
            noticeService.modifyNotice(notice);
            resultMap.put(CommonConstants.RESULT_STATUS, CommonConstants.SUCCESS_STATUS);
        } catch (FSException exc) {
            // 请保持所有代码一直性，没有特殊处理请使用该代码.
            this.printExction(resultMap, exc);
        }
        this.printLog(resultMap);
        return resultMap;
    }

    /**.
     *
     */
    @RequestMapping(value = "/topNotice", method = RequestMethod.POST)
    @OperationLog("置顶新闻公告")
    @ResponseBody
    public ModelMap topNotice(@RequestBody Notice notice) {
        ModelMap resultMap = new ModelMap();
        try {
            if (Tools.isEmpty(notice.getId())) {
                throw new FSException(NoticeException.TOPID_IS_NULL, "置顶对应ID不能为空");
            }
            if (Tools.isEmpty(notice.getTopStatus())) {
                throw new FSException(NoticeException.TOPSTATUS_IS_NULL, "置顶状态不能为空");
            }
            noticeService.modifyNotice(notice);
            resultMap.put(CommonConstants.RESULT_STATUS, CommonConstants.SUCCESS_STATUS);
        } catch (FSException exc) {
            // 请保持所有代码一直性，没有特殊处理请使用该代码.
            this.printExction(resultMap, exc);
        }
        this.printLog(resultMap);
        return resultMap;
    }

    /**.
     *
     */
    @RequestMapping(value = "/releaseNotice", method = RequestMethod.POST)
    @OperationLog("发布下架新闻公告")
    @ResponseBody
    public ModelMap releaseNotice(@RequestBody Notice notice) {
        ModelMap resultMap = new ModelMap();
        try {
            if (Tools.isEmpty(notice.getId())) {
                throw new FSException(NoticeException.RESID_IS_NULL, "对应ID不能为空");
            }
            if (Tools.isEmpty(notice.getStatus())) {
                throw new FSException(NoticeException.STATUS_IS_NULL, "发布状态不能为空");
            }
            if (Tools.isEmpty(notice.getReleaseUserId())) {
                throw new FSException(NoticeException.RESUSERID_IS_NULL, "发布人不能为空");
            }
            if (ArticleStatusEnum.RELEASE.toString().equals(notice.getStatus())) {
                notice.setReleaseTime(new Date(System.currentTimeMillis()));
                notice.setReleaseUserId(notice.getReleaseUserId());
            } else {
                notice.setReleaseUserId(null);
            }
            noticeService.modifyNotice(notice);
            resultMap.put(CommonConstants.RESULT_STATUS, CommonConstants.SUCCESS_STATUS);
        } catch (FSException exc) {
            // 请保持所有代码一直性，没有特殊处理请使用该代码.
            this.printExction(resultMap, exc);
        }
        this.printLog(resultMap);
        return resultMap;
    }


    /**
     * 前台调用，排序规则：发布状态（发布——未发布），置顶状态（置顶——非置顶），发布时间（倒序）.
     */
    @RequestMapping(value = "/queryNoticePage", method = RequestMethod.POST)
    @OperationLog("查询新闻公告(分页，前台)")
    @ResponseBody
    public ModelMap queryNoticePage(@RequestBody NoticeInfo notice) {
        ModelMap resultMap = new ModelMap();
        try {
            notice = noticeService.queryNoticePage(notice);
            resultMap.put(CommonConstants.RESULT_STATUS, CommonConstants.SUCCESS_STATUS);
            resultMap.put(CommonConstants.RESULT_DATA, notice);
        } catch (FSException exc) {
            // 请保持所有代码一直性，没有特殊处理请使用该代码.
            this.printExction(resultMap, exc);
        }
        this.printLog(resultMap);
        return resultMap;
    }

    /**
     * 后台调用，排序规则：更新时间（倒序）.
     */
    @RequestMapping(value = "/queryPage", method = RequestMethod.POST)
    @OperationLog("查询新闻公告(分页，后台)")
    @ResponseBody
    public ModelMap queryNotice(@RequestBody NoticeInfo notice) {
        ModelMap resultMap = new ModelMap();
        try {
            notice = noticeService.queryNotice(notice);
            resultMap.put(CommonConstants.RESULT_STATUS, CommonConstants.SUCCESS_STATUS);
            resultMap.put(CommonConstants.RESULT_DATA, notice);
        } catch (FSException excxc) {
            // 请保持所有代码一直性，没有特殊处理请使用该代码.
            this.printExction(resultMap, excxc);
        }
        this.printLog(resultMap);
        return resultMap;
    }

    /**.
     *
     */
    @RequestMapping(value = "/queryNotice", method = RequestMethod.POST)
    @OperationLog("查询新闻公告(单个)")
    @ResponseBody
    public ModelMap queryNotice(@RequestBody Notice notice) {
        ModelMap resultMap = new ModelMap();
        try {
            if (Tools.isEmpty(notice.getId())) {
                throw new FSException(NoticeException.QUEID_IS_NULL, "对应ID不能为空");
            }
            notice = noticeService.queryNotice(notice);
            resultMap.put(CommonConstants.RESULT_STATUS, CommonConstants.SUCCESS_STATUS);
            resultMap.put(CommonConstants.RESULT_DATA, notice);
        } catch (FSException excxc) {
            // 请保持所有代码一直性，没有特殊处理请使用该代码.
            this.printExction(resultMap, excxc);
        }
        this.printLog(resultMap);
        return resultMap;
    }

    /**.
     *
     */
    @RequestMapping(value = "/queryNoticeById", method = RequestMethod.POST)
    @OperationLog("查询单个(包含上一篇下一篇)")
    @ResponseBody
    public ModelMap queryNoticeById(@RequestBody Notice notice) {
        ModelMap resultMap = new ModelMap();
        try {
            if (Tools.isEmpty(notice.getId())) {
                throw new FSException(BannerExceptin.QUEID_IS_NULL, "对应ID不能为空");
            }
            notice = noticeService.queryNoticeById(notice);
            resultMap.put(CommonConstants.RESULT_STATUS, CommonConstants.SUCCESS_STATUS);
            resultMap.put(CommonConstants.RESULT_DATA, notice);
        } catch (FSException exc) {
            // 请保持所有代码一直性，没有特殊处理请使用该代码.
            this.printExction(resultMap, exc);
        }
        this.printLog(resultMap);
        return resultMap;
    }

    /**
     * 前台调用，排序规则：发布状态（发布——未发布），置顶状态（置顶——非置顶），发布时间（倒序).
     */
    @RequestMapping(value = "/queryNoticeAndImg", method = RequestMethod.POST)
    @OperationLog("查询新闻公告(分页，前台)")
    @ResponseBody
    public ModelMap queryNoticeAndImg(@RequestBody NoticeInfo notice) {
        ModelMap resultMap = new ModelMap();
        try {
            notice = noticeService.queryNoticeAndImg(notice);
            List<Notice> list = new ArrayList();
            resultMap.put(CommonConstants.RESULT_STATUS, CommonConstants.SUCCESS_STATUS);
            resultMap.put(CommonConstants.RESULT_DATA, notice);
        } catch (FSException exc) {
            // 请保持所有代码一直性，没有特殊处理请使用该代码.
            this.printExction(resultMap, exc);
        }
        this.printLog(resultMap);
        return resultMap;
    }

    @Override
    protected void printLog(Object data) {
        List<ServiceInstance> instances = client.getInstances(instanceId);
        for (ServiceInstance instance : instances) {
            logger.info("/add, host:" + instance.getHost() + ", service_id:" + instance.getServiceId() + ", result:"
                    + data);
        }
    }
    
}
