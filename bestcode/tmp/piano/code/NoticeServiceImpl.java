/*
 * 文件名：NoticeServiceImpl.java
 * 版权：Copyright by www.fsmeeting.com
 * 描述：
 * 修改人：pich
 * 修改时间：2017年5月12日
 * 修改内容：
 */

package com.fs.party.article.service.impl;

import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import com.fs.party.article.common.ArticleStatusEnum;
import com.fs.party.article.common.ArticleTopEnum;
import com.fs.party.article.common.ArticleTypeEnum;
import com.fs.party.article.dao.mybatis.party.INoticeMapper;
import com.fs.party.article.exception.NoticeException;
import com.fs.party.article.pojo.Notice;
import com.fs.party.article.pojo.vo.NoticeInfo;
import com.fs.party.article.service.INoticeService;
import com.fs.party.article.service.ISystemClient;
import com.fs.party.depeds.base.exception.FSException;
import com.fs.party.depeds.base.service.BaseServiceImpl;
import com.fs.party.depeds.pojo.UserPartyInfo;
import com.fs.party.depeds.utils.Tools;
import com.github.pagehelper.PageHelper;
import com.github.pagehelper.PageInfo;

@Service
public class NoticeServiceImpl extends BaseServiceImpl implements INoticeService {
    
    @Autowired
    private INoticeMapper noticeMapper;
  
    @Autowired
    private ISystemClient systemClient;
    
    @Override
    public void addNotice(Notice notice) {
        try {
            //查询是否存在该用户 ,是否存在该组织
            /*if (noticeMapper.queryUserExist(notice) == 0) {
                throw new FSException(NoticeException.USER_IS_NOT_EXIST, "该用户不存在");
            }else if (noticeMapper.queryOrgExist(notice) == 0) {
                throw new FSException(NoticeException.ORG_IS_NOT_EXIST, "所属组织不存在");
            }*/
            if (notice.getTitle().getBytes().length > 90) {
                throw new FSException(NoticeException.TITLELEN_IS_LONG, "标题长度超过限制");
            }
            if (!Tools.isEmpty(notice.getType())) {
                if (!ArticleTypeEnum.CITYNEWS.toString().equals(notice.getType())
                        && !ArticleTypeEnum.CADREWORK.toString().equals(notice.getType())
                        && !ArticleTypeEnum.INFORMATIONRELEASE.toString().equals(notice.getType())
                        && !ArticleTypeEnum.PARTYBUILDING.toString().equals(notice.getType())
                        && !ArticleTypeEnum.PARTYKNOWLEDGE.toString().equals(notice.getType())
                        && !ArticleTypeEnum.POLREG.toString().equals(notice.getType())
                        && !ArticleTypeEnum.TALENT.toString().equals(notice.getType())
                        && !ArticleTypeEnum.TISSUEGARDEN.toString().equals(notice.getType())
                        && !ArticleTypeEnum.VILLAGEAPP.toString().equals(notice.getType())
                        && !ArticleTypeEnum.NOTICE.toString().equals(notice.getType())) {
                    throw new FSException(NoticeException.TYPE_IS_ILLEGAL, "类型错误");
                }
            }
            notice.setCreateTime(new Date(System.currentTimeMillis()));
            notice.setModifyTime(new Date(System.currentTimeMillis()));
            //发布状态为暂存，置顶状态为非置顶
            notice.setStatus(ArticleStatusEnum.ALPHA.toString());
            notice.setTopStatus(ArticleTopEnum.DOWN.toString());
            //notice.setType(ArticleTypeEnum.NOTICE.toString());
            noticeMapper.insert(notice);
        } catch (Throwable thr) {
            throw this.catchException(thr);
        }
    }

    @Override
    public void delNotice(String ids) {
        try {
            String[] idStr = ids.split(",");
            for (String noticeId : idStr) {
                if (!noticeId.matches("^[0-9]+$")) {
                    throw new FSException(NoticeException.ID_IS_ILLEGAL, "Id只能为数字");
                }
                Notice notice = new Notice();
                notice.setId(Long.parseLong(noticeId));
                if (noticeMapper.queryNotById(notice) == 0) {
                    throw new FSException(NoticeException.NOTICEID_IS_NOT_EXIST, "该通知公告不存在");
                }
                if (!Tools.isEmpty(noticeId)) {
                    noticeMapper.del(Long.valueOf(noticeId));
                }
            }
        } catch (Throwable thr) {
            throw this.catchException(thr);
        }
    }

    @Override
    public void modifyNotice(Notice notice) {
        try {
            if (noticeMapper.queryNotById(notice) == 0) {
                throw new FSException(NoticeException.NOTICEID_IS_NOT_EXIST, "该通知公告不存在");
            }
            if (!Tools.isEmpty(notice.getTitle())) {
                if (notice.getTitle().getBytes().length > 90) {
                    throw new FSException(NoticeException.TITLELEN_IS_LONG, "标题长度超过限制");
                }
            }
            if (!Tools.isEmpty(notice.getStatus())) {
                if (!ArticleStatusEnum.ALPHA.toString().equals(notice.getStatus())
                        && !ArticleStatusEnum.RELEASE.toString().equals(notice.getStatus())) {
                    throw new FSException(NoticeException.STATUS_IS_ILLEGAL, "发布状态只能为ALPHA或RELEASE");
                } else {
                    //下架时不修改发布时间
                    if (!ArticleStatusEnum.ALPHA.toString().equals(notice.getStatus())) {
                        notice.setReleaseTime(new Date(System.currentTimeMillis()));
                    }
                }
            }
            if (!Tools.isEmpty(notice.getTopStatus())) {
                if (!notice.getTopStatus().equals(ArticleTopEnum.TOP.toString())
                        && !notice.getTopStatus().equals(ArticleTopEnum.DOWN.toString())) {
                    throw new FSException(NoticeException.STATUS_IS_ILLEGAL, "置顶状态只能为TOP或DOWN");
                }
            }
            notice.setModifyTime(new Date(System.currentTimeMillis()));
            noticeMapper.update(notice);
        } catch (Throwable thr) {
            throw this.catchException(thr);
        }
    }

    @Override
    public NoticeInfo queryNoticePage(NoticeInfo notice) {
        List<Notice> list = new ArrayList<Notice>();
        try {
            PageHelper.startPage(notice.getCurrentPage(),notice.getPageSize());
            list = noticeMapper.querySelective(notice);
            for (Notice notice2 : list) {
                if (!Tools.isEmpty(notice2.getReleaseUserId())) {
                    UserPartyInfo user = new UserPartyInfo();
                    user.setUserId(notice2.getReleaseUserId());
                    user = systemClient.queryUserById(user);
                    if (!Tools.isEmpty(user)) {
                        notice2.setReleaseUserName(user.getName());
                    }
                }
            }
            PageInfo<Notice> pageInfo = new PageInfo<Notice>(list);
            notice.setData(pageInfo.getList());
            notice.setTotalRecords((int) pageInfo.getTotal());
            return notice;
        } catch (Throwable thr) {
            throw this.catchException(thr);
        }
    }

    @Override
    public Notice queryNotice(Notice notice) {
        try {
            if (noticeMapper.queryNotById(notice) == 0) {
                throw new FSException(NoticeException.NOTICEID_IS_NOT_EXIST, "该通知公告不存在");
            }
            notice = noticeMapper.queryNoticeBack(notice);
            /*if (!Tools.isEmpty(notice.getReleaseUserId())) {
                UserPartyInfo user = new UserPartyInfo();
                user.setUserId(notice.getReleaseUserId());
                user = systemClient.queryUserById(user);
                if (!Tools.isEmpty(user) &&!Tools.isEmpty(user.getName())) {
                    notice.setReleaseUserName(user.getName());
                }
            }*/
            return notice;
        } catch (Throwable thr) {
            throw this.catchException(thr);
        }
    }

    @Override
    public NoticeInfo queryNotice(NoticeInfo notice) {
        List<Notice> list = new ArrayList<Notice>();
        try {
            if (!Tools.isEmpty(notice.getStatus())) {
                if (!ArticleStatusEnum.ALPHA.toString().equals(notice.getStatus())
                        && !ArticleStatusEnum.RELEASE.toString().equals(notice.getStatus())) {
                    throw new FSException(NoticeException.STATUS_IS_ILLEGAL, "发布状态只能为ALPHA或RELEASE");
                }
            }
            PageHelper.startPage(notice.getCurrentPage(),notice.getPageSize());
            list = noticeMapper.query(notice);
            for (Notice notice2 : list) {
                if (!Tools.isEmpty(notice2.getReleaseUserId())) {
                    UserPartyInfo user = new UserPartyInfo();
                    user.setUserId(notice2.getReleaseUserId());
                    user = systemClient.queryUserById(user);
                    if (!Tools.isEmpty(user)) {
                        notice2.setReleaseUserName(user.getName());
                    }
                }
            }
            PageInfo<Notice> pageInfo = new PageInfo<Notice>(list);
            notice.setData(pageInfo.getList());
            notice.setTotalRecords((int) pageInfo.getTotal());
            return notice;
        } catch (Throwable thr) {
            throw this.catchException(thr);
        }
    }

    @Override
    public Notice queryNoticeById(Notice notice) {
        try {
            String type = "";
            if (noticeMapper.queryNotById(notice) == 0) {
                throw new FSException(NoticeException.NOTICEID_IS_NOT_EXIST, "该通知不存在");
            }
            int row = noticeMapper.getrow(notice);
            int preRow = row - 1;

            notice = noticeMapper.queryNotice(notice);
            if (!Tools.isEmpty(notice)) {
                type = notice.getType();
            }
            if (!Tools.isEmpty(notice) && !Tools.isEmpty(noticeMapper.queryNoticeRow(preRow,type))) {
                notice.setPrefer(noticeMapper.queryNoticeRow(preRow,type));
            }
            int  nextRow = row + 1;
            if (!Tools.isEmpty(notice) && !Tools.isEmpty(noticeMapper.queryNoticeRow(nextRow,type))) {
                notice.setAfter(noticeMapper.queryNoticeRow(nextRow,type));
            }
            return notice;
        } catch (Throwable thr) {
            throw this.catchException(thr);
        }
    
    }

    @Override
    public NoticeInfo queryNoticeAndImg(NoticeInfo notice) {
        List<Notice> list = new ArrayList<Notice>();
        try {
            PageHelper.startPage(notice.getCurrentPage(),notice.getPageSize());
            list = noticeMapper.querySelective(notice);
            for (Notice notice2 : list) {
                if (!Tools.isEmpty(notice2.getReleaseUserId())) {
                    UserPartyInfo user = new UserPartyInfo();
                    user.setUserId(notice2.getReleaseUserId());
                    user = systemClient.queryUserById(user);
                    if (!Tools.isEmpty(user)) {
                        notice2.setReleaseUserName(user.getName());
                    }
                }
            }
            //获取图片地址
            for (Notice notice1 : list) {
                List<String> imgList = new ArrayList<String>();
                imgList = Tools.getImgStr(notice1.getContent());
                notice1.setImgList(imgList);
            }
            PageInfo<Notice> pageInfo = new PageInfo<Notice>(list);
            notice.setData(pageInfo.getList());
            notice.setTotalRecords((int) pageInfo.getTotal());
            return notice;
        } catch (Throwable thr) {
            throw this.catchException(thr);
        }
    }
}
