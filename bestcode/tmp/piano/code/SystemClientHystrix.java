/*
 * 文件名：ISystemClientHystrix.java
 * 版权：Copyright by www.fsmeeting.com
 * 描述：
 * 修改人：pich
 * 修改时间：2017年6月8日
 * 修改内容：
 */

package com.fs.party.article.service.impl;

import com.fs.party.article.service.ISystemClient;
import com.fs.party.depeds.pojo.UserPartyInfo;
import org.springframework.stereotype.Component;

@Component
public class SystemClientHystrix implements ISystemClient {
    @Override
    public UserPartyInfo queryUserById(UserPartyInfo user) {
        UserPartyInfo userPartyInfo = new UserPartyInfo();
        return userPartyInfo;
    }

    @Override
    public String queryOrganizationName(Integer administrativeId) {
        String organizationName = "获取失败";
        return organizationName;
    }
}
