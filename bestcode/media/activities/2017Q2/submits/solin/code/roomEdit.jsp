<%@ page language="java" contentType="text/html; charset=UTF-8"
	pageEncoding="UTF-8"%>
<%@ taglib prefix="c" uri="http://java.sun.com/jsp/jstl/core" %>
<%@taglib prefix="fmt" uri="http://java.sun.com/jsp/jstl/fmt" %>
<%
    String ctxPath=request.getContextPath();
%>
<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
<title><fmt:message key="title.room.addroomtitle" /></title>
<script type="text/javascript">
var room = ${room};
var subDepartList = ${subDepartList};
var maxVideoStr = ${maxVideo};
</script>
</head>
<body>

<form id="formEdit">
    <div class="modal-dialog modal-lg" role="document">
        <div class="modal-content">
            <div class="modal-header">
                <button type="button" class="close" data-dismiss="modal" aria-label="Close"><span aria-hidden="true">&times;</span></button>
                <h4 class="modal-title" id="myModalLabel">
                <!-- ko if: editModel.roomID() == 0-->
                <fmt:message key="title.room.addroomtitle"/>
                 <!-- /ko -->
                 <!-- ko if: editModel.roomID() != 0-->
                <fmt:message key="title.room.editroomtitle"/>
                <!-- /ko -->
                </h4>
            </div>
            
            <div class="modal-body">
    
                <legend>会议基本信息</legend>  
                <div class="row LineSpacing">
                    <label for="txt_roomName" class="col-md-2 col-right"><fmt:message key="menu.roomadd.roomname" />：</font></label>
                    <div class="col-md-4"><input type="text" name="txt_roomName" data-bind="value:editModel.roomName" class="form-control inputMandatory" placeholder='<fmt:message key="menu.roomadd.roomname" />'></div>
                     <label for="txt_maxUserCount" class="col-md-2 col-right"><fmt:message key="menu.roomadd.maxusercount" />：</label>
                    <div class="col-md-4"><input type="text" name="txt_maxUserCount" data-bind="value:editModel.maxUserCount" class="form-control inputMandatory" placeholder='<fmt:message key="menu.roomadd.maxusercount" />'></div>
                </div>
                <div class="row LineSpacing">
                    <label for="txt_enableChairPwd" class="col-md-2"><font size="1px"><fmt:message key="menu.roomadd.enablechairpwd" />：</font></label>
                    <div class="col-md-4">
   						 <input type="radio" name="enableChairPwd" value="1" data-bind="checked: editModel.enableChairPwd, event:{change:changeChairPasswordStatus}" />是
   						 <input type="radio" name="enableChairPwd" value="0" data-bind="checked: editModel.enableChairPwd, event:{change:changeChairPasswordStatus}" />否
					</div>
                    
                    <label for="txt_chairPassword" class="col-md-2"><fmt:message key="menu.roomadd.chairpassword" />：</label>
                    <div class="col-md-4"><input type="password" id="pwd_chairPassword" name="txt_chairPassword" data-bind="value:editModel.chairPassword" class="form-control" placeholder='<fmt:message key="menu.roomadd.chairpassword" />' disabled="disabled"></div>
                </div>
                <div class="row LineSpacing">
                    <label for="txt_verifyMode" class="col-md-2"><fmt:message key="menu.roomadd.verifymode" />：</label>
                    <div class="col-md-4">
					    <select class="form-control" data-bind="options:selectData.verifyModeData.items,optionsText:'name',optionsValue:'id',value:editModel.verifyMode, event:{change:changePasswordStatus}"></select>
					</div>
                    
                    <label for="txt_password" class="col-md-2"><fmt:message key="menu.roomadd.password" />：</label>
                    <div class="col-md-4"><input type="password" id="pwd_password" name="txt_password" data-bind="value:editModel.password" class="form-control" placeholder='<fmt:message key="menu.roomadd.password" />' disabled="disabled"></div>
                </div>
                <div class="row LineSpacing">
                    <label for="txt_parentdepart" class="col-md-2"><fmt:message key="menu.roomadd.parentdepart" />：</label>
                    <div class="col-md-4">
					    <select class="form-control" data-bind="options:selectData.parent.items,optionsText:'departName',optionsValue:'departID',value:editModel.departID"></select>
					</div>
	                <label for="txt_roomStatus" class="col-md-2"><fmt:message key="menu.roomadd.roomstatus" />：</label>
	                <div class="col-md-4">
				    	<select class="form-control" data-bind="options:selectData.roomStatusData.items,optionsText:'name',optionsValue:'id',value:editModel.status"></select>
					</div>
				</div>
				<div class="row LineSpacing">
                    <label for="txt_roomDesc" class="col-md-2"><fmt:message key="menu.roomadd.roomDesc" />：</label>
                    <div class="col-md-10">
					    <textarea class="form-control" rows="2" name="txt_roomDesc" data-bind="value:editModel.roomDesc"  placeholder='<fmt:message key="menu.roomadd.roomDesc" />'></textarea>
					</div>
				</div>
				<!-- ko if: editModel.roomID() != 0-->
				<div class="row LineSpacing">
                    <label for="txt_userLoginUrl" class="col-md-2"><fmt:message key="menu.roomedit.userloginurl" />：</label>
                    <div class="col-md-10">
					    <input class="form-control"  name="txt_userLoginUrl" value="${basePath}" readonly="readonly"/>
					</div>
				</div>
				<div class="row LineSpacing">
                    <label for="txt_anonymousLoginUrl" class="col-md-2"><fmt:message key="menu.roomedit.anonymousloginurl" />：</label>
                    <div class="col-md-10">
					    <input class="form-control"  name="txt_anonymousLoginUrl" value="${basePath}&userType=0" readonly="readonly"/>
					</div>
				</div>
				<!-- /ko -->
				<legend>会议类型设置</legend>  
				<div class="row LineSpacing">
                    <label for="txt_roomType" class="col-md-2"><fmt:message key="menu.roomadd.roomtype" />：</label>
                    <div class="col-md-4">
					    <select class="form-control" data-bind="options:selectData.roomTypeModelData.items,optionsText:'name',optionsValue:'id',value:editModel.roomType"></select>
					</div>
               	</div>
               	<div data-bind="visible: editModel.roomType() == 2">
	               	<div class="row LineSpacing">
	                    <label for="txt_hopeStartTime" class="col-md-2"><fmt:message key="menu.roomadd.hopeStartTime" />：</label>
	                    <div class="col-md-4">
		                	<input type="text" name="txt_hopeStartTime" onfocus="WdatePicker({onpicked:pickedhopeStartTime,skin:'twoer',minDate:'%y-%M-%d',dateFmt:'yyyy-MM-dd HH:mm:ss', isShowClear:true, readOnly:true,lang:'${pageContext.response.locale}'})" data-bind="value:editModel.hopeStartTime" class="form-control Wdate DateHeight" placeholder='<fmt:message key="menu.roomadd.hopeStartTime" />'>
						</div>
						 <label for="txt_hopeEndTime" class="col-md-2"><fmt:message key="menu.roomadd.hopeEndTime" />：</label>
	                    <div class="col-md-4">
		                	<input type="text" onfocus="WdatePicker({onpicked:pickedhopeEndTime,skin:'twoer',minDate:'%y-%M-%d',dateFmt:'yyyy-MM-dd HH:mm:ss', isShowClear:true, readOnly:true,lang:'${pageContext.response.locale}'})" name="txt_hopeEndTime" data-bind="value:editModel.hopeEndTime" class="form-control Wdate DateHeight" placeholder='<fmt:message key="menu.roomadd.hopeEndTime" />'>
						</div>
	               	</div>
               	</div>
               	<div data-bind="visible: editModel.roomType() == 4">
	               	<div class="row LineSpacing">
	                    <label for="txt_weekStartTime" class="col-md-2"><fmt:message key="menu.roomadd.weekStartTime" />：</label>
	                    <div class="col-md-4">
		                	<input type="text" onfocus="WdatePicker({onpicked:pickedweekStartTime,skin:'twoer',dateFmt:'HH:mm:ss', isShowClear:true, readOnly:true,lang:'${pageContext.response.locale}'})" name="txt_weekStartTime" data-bind="value:editModel.weekStartTime" class="form-control Wdate DateHeight" placeholder='<fmt:message key="menu.roomadd.weekStartTime" />'>
						</div>
						 <label for="txt_weekEndTime" class="col-md-2"><fmt:message key="menu.roomadd.weekEndTime" />：</label>
	                    <div class="col-md-4">
		                	<input type="text" onfocus="WdatePicker({onpicked:pickedweekEndTime,skin:'twoer',dateFmt:'HH:mm:ss', isShowClear:true, readOnly:true,lang:'${pageContext.response.locale}'})" name="txt_weekEndTime" data-bind="value:editModel.weekEndTime" class="form-control Wdate DateHeight" placeholder='<fmt:message key="menu.roomadd.weekEndTime" />'>
						</div>
	               	</div>
	               	<div class="row LineSpacing">
	               		<label for="txt_cycleFlagSelect" class="col-md-2"><fmt:message key="menu.roomadd.cycleFlagSelect" />：</label>
		               	<div class="col-md-12">
					    	<input type="radio" name="cycleFlag" value="1" data-bind="checked: editModel.cycleFlag ,event:{change:changeCycleFlagStatus}" />每天
						</div>
					</div>
					<div class="row LineSpacing">
						<div class="col-md-2">
					    	<input type="radio" name="cycleFlag" value="2" data-bind="checked: editModel.cycleFlag,event:{change:changeCycleFlagStatus}" />每周
					    </div>
					    <div class="col-md-10" data-bind="visible: editModel.cycleFlag() == 2">
					    	<input type="checkbox" name="weeks"  value="0" data-bind="checked: editModel.weeks" />星期一
					    	<input type="checkbox" name="weeks"  value="1" data-bind="checked: editModel.weeks" />星期二
					    	<input type="checkbox" name="weeks"  value="2" data-bind="checked: editModel.weeks" />星期三
					    	<input type="checkbox" name="weeks"  value="3" data-bind="checked: editModel.weeks" />星期四
					    	<input type="checkbox" name="weeks"  value="4" data-bind="checked: editModel.weeks" />星期五
					    	<input type="checkbox" name="weeks"  value="5" data-bind="checked: editModel.weeks" />星期六
					    	<input type="checkbox" name="weeks"  value="6" data-bind="checked: editModel.weeks" />星期七
						</div>
					</div>
					<div class="row LineSpacing">
						<div class="col-md-2">
					    	<input type="radio" name="cycleFlag" value="3" data-bind="checked: editModel.cycleFlag ,event:{change:changeCycleFlagStatus}" />每月
					    </div>
					    <div class="col-md-10" data-bind="visible: editModel.cycleFlag() == 3">
					    	<input type="text" name="txt_dateEveyMonth" data-bind="value:editModel.dateEveyMonth" class="form-control" style="width: 70px"  placeholder='几号'>
						</div>
	               	</div>
               	</div>
               	<legend>会议视频参数</legend>  
               	<div class="row LineSpacing">
                    <label for="txt_useDefaultFlag" class="col-md-2"><font size="1px"><fmt:message key="menu.roomadd.isusedefaultflag" />：</font></label>
                    <div class="col-md-4">
					     <input type="radio" name="isusedefaultflag" value="1" data-bind="checked: editModel.useDefaultFlag, event:{change:changeDefaultSetStatus}" />是
   						 <input type="radio" name="isusedefaultflag" value="0" data-bind="checked: editModel.useDefaultFlag, event:{change:changeDefaultSetStatus}" />否
					</div>
            	</div>
				<div class="row LineSpacing" data-bind="visible: editModel.useDefaultFlag() == 1">
                    <label for="txt_defaultVideoCodec" class="col-md-2"><fmt:message key="menu.roomadd.defaultvideocodec" />：</label>
                    <div class="col-md-4">
					    <select class="form-control" data-bind="options:selectData.defaultVideoCodecData.items,optionsText:'name',optionsValue:'id',value:editModel.defaultVideoCodec"></select>
					</div>
					<label for="txt_defaultVideoWind" class="col-md-2"><fmt:message key="menu.roomadd.defaultvideowind" />：</label>
                    <div class="col-md-4">
					    <select class="form-control" data-bind="options:selectData.defaultVideoWindData.items,optionsText:'name',optionsValue:'id',value:editModel.defaultVideoWind"></select>
					</div>
               	</div>
               	<div class="row LineSpacing" data-bind="visible: editModel.useDefaultFlag() == 1">
                    <label for="txt_defaultVideoBitrate" class="col-md-2"><fmt:message key="menu.roomadd.defaultvideobitrate" />：</label>
                    <div class="col-md-4"><input type="text" id="id_defaultVideoBitrate" name="txt_defaultVideoBitrate" data-bind="value:editModel.defaultVideoBitrate" class="form-control" placeholder='<fmt:message key="menu.roomadd.defaultvideobitrate" />'></div>
               	</div>
				<legend>会议权限设置</legend>  
				<div class="row LineSpacing">
                    <label for="txt_video" class="col-md-2"><fmt:message key="menu.roomadd.video" />：</label>
                    <div class="col-md-2">
   						 <input type="checkbox" name="autoBrdVideo"  value="true" data-bind="checked: editModel.autoBrdVideo" /><fmt:message key="menu.roomadd.autobrdvideo" />
   					 </div>
   					 <div class="col-md-2">
   						 <select class="form-control" data-bind="options:selectData.meetingRoleData.items,optionsText:'name',optionsValue:'id',value:editModel.autoBrdVideoPerm"></select>
					</div>
					
					<label for="txt_audio" class="col-md-2"><fmt:message key="menu.roomadd.audio" />：</label>
                    <div class="col-md-2">
   						 <input type="checkbox" name="autoBrdAudio"  value="true" data-bind="checked: editModel.autoBrdAudio" /><fmt:message key="menu.roomadd.autobrdaudio" />
   					 </div>
   					 <div class="col-md-2">
   						 <select class="form-control" data-bind="options:selectData.meetingRoleData.items,optionsText:'name',optionsValue:'id',value:editModel.autoBrdAudioPerm"></select>
					</div>
            	</div>
            	
            	<div class="row LineSpacing">
                    <label for="txt_chat" class="col-md-2"><fmt:message key="menu.roomadd.chat" />：</label>
                    <div class="col-md-2">
   						 <input type="checkbox" name="enablePubChat"  value="true" data-bind="checked: editModel.enablePubChat" /><fmt:message key="menu.roomadd.enablePubChat" />
   					 </div>
   					 <div class="col-md-2">
   						 <input type="checkbox" name="enableP2PChat"  value="true" data-bind="checked: editModel.enableP2pChat" /><fmt:message key="menu.roomadd.enableP2PChat" />
					</div>
					
					<label for="txt_interface" class="col-md-2"><fmt:message key="menu.roomadd.interface" />：</label>
                    <div class="col-md-4">
   						 <input type="checkbox" name="enableKeepVideo"  value="true" data-bind="checked: editModel.enableKeepVideo" /><fmt:message key="menu.roomadd.enableKeepVideo" />
   					 </div>
            	</div>
            	
            	<div class="row LineSpacing">
                    <label for="txt_whiteboard" class="col-md-2"><fmt:message key="menu.roomadd.whiteboard" />：</label>
                    <div class="col-md-2">
   						 <input type="checkbox" name="autoBrdWB"  value="true" data-bind="checked: editModel.autoBrdWb" /><fmt:message key="menu.roomadd.autoBrdWB" />
   					 </div>
   					 <div class="col-md-2">
   						 <select class="form-control" data-bind="options:selectData.meetingRoleData.items,optionsText:'name',optionsValue:'id',value:editModel.autoBrdWbPerm"></select>
					</div>
					
                    <div class="col-md-6">
   						 <input type="checkbox" name="serverEnableWB"  value="true" data-bind="checked: editModel.serverEnableWb" /><fmt:message key="menu.roomadd.serverEnableWB" />
   					 </div>
            	</div>
            	<div class="row LineSpacing">
                    <label for="txt_record" class="col-md-2"><fmt:message key="menu.roomadd.record" />：</label>
                    <div class="col-md-4">
   						 <input type="checkbox" name="enableRecord"  value="true" data-bind="checked: editModel.enableRecord" /><fmt:message key="menu.roomadd.enableRecord" />
   					 </div>
					
					<label for="txt_file" class="col-md-2"><fmt:message key="menu.roomadd.file" />：</label>
                    <div class="col-md-4">
   						 <input type="checkbox" name="enableFile"  value="true" data-bind="checked: editModel.enableFile" /><fmt:message key="menu.roomadd.enableFile" />
   					 </div>
            	</div>
            </div>
            
            <div class="modal-footer">
                <button type="button" class="btn btn-default" data-dismiss="modal"><span class="glyphicon glyphicon-remove" aria-hidden="true"></span><fmt:message key="menu.common.closebtn" /></button>
                <button type="submit" class="btn btn-primary"><span class="glyphicon glyphicon-floppy-disk" aria-hidden="true"></span><fmt:message key="menu.common.savabtn" /></button>
            </div>
        </div>
    </div>
</form>
</body>

<script type="text/javascript" src="<%=ctxPath%>/resources/plugin/My97DatePicker/WdatePicker.js"></script>
<script type="text/javascript" src="<%=ctxPath%>/resources/fmWeb/room/js/roomEdit.js"></script>
<link href="<%=ctxPath%>/resources/fmWeb/room/css/roomEdit.css" rel="stylesheet" type="text/css"/> 
</html>