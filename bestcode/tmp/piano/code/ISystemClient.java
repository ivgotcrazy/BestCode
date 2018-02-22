/*
 * 文件名：ISystemClient.java
 * 版权：Copyright by www.fsmeeting.com
 * 描述：
 * 修改人：pich
 * 修改时间：2017年6月8日
 * 修改内容：
 */

package com.fs.party.article.service;

import com.fs.party.article.service.impl.SystemClientHystrix;
import com.fs.party.depeds.pojo.UserPartyInfo;
import org.springframework.cloud.netflix.feign.FeignClient;
import org.springframework.web.bind.annotation.RequestBody;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestMethod;
import org.springframework.web.bind.annotation.RequestParam;

@FeignClient(value = "system-service", fallback = SystemClientHystrix.class)
public interface ISystemClient {
    
    @RequestMapping(value = "/api/queryUserById",method = RequestMethod.POST)    
    public UserPartyInfo queryUserById(@RequestBody UserPartyInfo user);

    @RequestMapping(value = "/api/queryOrganization",method = RequestMethod.GET)
    public String queryOrganizationName(@RequestParam(value = "administrativeId") Integer administrativeId);
}
