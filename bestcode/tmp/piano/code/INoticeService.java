/*
 * 文件名：INoticeService.java
 * 版权：Copyright by www.fsmeeting.com
 * 描述：
 * 修改人：pich
 * 修改时间：2017年5月12日
 * 修改内容：
 */

package com.fs.party.article.service;

import com.fs.party.article.pojo.Notice;
import com.fs.party.article.pojo.vo.NoticeInfo;

public interface INoticeService {

    void addNotice(Notice notice);

    void delNotice(String ids);

    void modifyNotice(Notice notice);

    NoticeInfo queryNoticePage(NoticeInfo notice);
    
    NoticeInfo queryNotice(NoticeInfo notice);
    
    Notice queryNotice(Notice notice);
    
    Notice queryNoticeById(Notice notice);

    NoticeInfo queryNoticeAndImg(NoticeInfo notice);
}
