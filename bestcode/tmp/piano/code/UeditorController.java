/*
 * 文件名：UeditorController.java
 * 版权：Copyright by www.fsmeeting.com
 * 描述：
 * 修改人：pich
 * 修改时间：2017年6月12日
 * 修改内容：
 */

package com.fs.party.article.controller.api;

import java.io.File;
import java.io.IOException;
import java.io.PrintWriter;
import java.util.Date;

import javax.servlet.http.HttpServletRequest;
import javax.servlet.http.HttpServletResponse;

import org.apache.log4j.Logger;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.RestController;
import org.springframework.web.multipart.MultipartFile;

import com.fs.party.depeds.date.DateFormatString;
import com.fs.party.depeds.date.DateTools;

@RestController
@RequestMapping(value = "/ckeditor")
public class UeditorController
{
    private final Logger logger = Logger.getLogger(getClass());
    
    
    @Value("${ueditor_dir}")
    private String uploadDir;
    
    @Value("${image_root_url}")
    private String imageRootUrl;
    
    @RequestMapping(value = "/uploadImg")
    public void upload(HttpServletRequest request, HttpServletResponse response,
            @RequestParam("upload") MultipartFile multipartFile)
            throws IOException
    {
        response.setContentType("text/html; charset=UTF-8");
        response.setHeader("Cache-Control", "no-cache");
        logger.info("imag uploading");
        String path = this.uploadDir + "/";
        String suffix = multipartFile.getOriginalFilename().substring(multipartFile.getOriginalFilename().lastIndexOf(".")).toLowerCase();
        String fileName = "image" + "/"
                + DateTools.dateToString(new Date(),
                        DateFormatString.yyyy_MM_dd)
                + "/" + System.currentTimeMillis()
                + suffix;
        File targetFile = new File(path, fileName);
        if (!targetFile.getParentFile().exists())
        {
            targetFile.getParentFile().mkdirs();
        }
        try
        {
            multipartFile.transferTo(targetFile);
        } catch (Exception e)
        {
            logger.error(e);
        }
        String callback = request.getParameter("CKEditorFuncNum");
        String backUrl = request.getParameter("backUrl");
        String message = "";
        response.sendRedirect(backUrl+"?ImageUrl="+ this.imageRootUrl + fileName + "&Message=" + message + "&CKEditorFuncNum=" + callback); 
    }
}
