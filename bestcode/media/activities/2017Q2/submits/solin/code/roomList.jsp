<%@ page language="java" contentType="text/html; charset=UTF-8"
	pageEncoding="UTF-8"%>
<%@ taglib prefix="c" uri="http://java.sun.com/jsp/jstl/core" %>
<%@taglib prefix="fmt" uri="http://java.sun.com/jsp/jstl/fmt" %>
<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html>
<head>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
<title><fmt:message key="title.room.roomlisttitle"/></title>
<%@include file="../other/include.jsp"%>

<link rel="stylesheet" type="text/css" href="<%=ctxPath%>/resources/plugin/bootstrap/table/bootstrap-table.min.css"/>
<script type="text/javascript" src="<%=ctxPath%>/resources/plugin/bootstrap/table/bootstrap-table.min.js"></script>
<!-- 根据语言类型，分页组件动态加载语言文件，默认是英文 -->
<c:if test="${pageContext.response.locale=='zh_CN'}">
	<script type="text/javascript" src="<%=ctxPath%>/resources/plugin/bootstrap/table/lang/bootstrap-table-zh-CN.min.js"></script>
</c:if>
<c:if test="${pageContext.response.locale=='zh_TW'}">
	<script type="text/javascript" src="<%=ctxPath%>/resources/plugin/bootstrap/table/lang/bootstrap-table-zh-TW.min.js"></script>
</c:if>
<script type="text/javascript" src="<%=ctxPath%>/resources/plugin/knockout/knockout.index.js"></script>

<script src="<%=ctxPath%>/resources/plugin/zTree/3.5/jquery.ztree.core-3.5.js" type="text/javascript"></script>  
<link href="<%=ctxPath%>/resources/plugin/zTree/3.5/zTreeStyle.css" rel="stylesheet" type="text/css"/> 

<script type="text/javascript">
var rootDepartFlag = ${sessionScope.rootDepartFlag};
var localNodeID= ${sessionScope.nodeID};
</script>
<script type="text/javascript" src="<%=ctxPath%>/resources/fmWeb/other/js/departTree.js"></script>
<script type="text/javascript" src="<%=ctxPath%>/resources/fmWeb/room/js/roomList.js"></script>
<link href="<%=ctxPath%>/resources/plugin/knockout/bootstrapValidator.css" rel="stylesheet" />
<script  type="text/javascript" src="<%=ctxPath%>/resources/plugin/knockout/bootstrapValidator.js"></script>
<script type="text/javascript" src="<%=ctxPath%>/resources/plugin/knockout/knockout.edit.js"></script>
</head>
<body style="overflow:auto;">

<div>
	<div class="panel-body col-xs-2" style="position: absolute;padding:0px;min-height:inherit;
	padding-right:5px;margin-bottom: 5px;top:0;bottom:0;">
		 
        <%@ include file="../other/departTree.jsp"%>
        
	</div>
	<div id="div_index" class="panel-body col-xs-10" style="padding:0px;overflow-x:hidden;margin-right:0;float:right">
        <div class="panel panel-default">
            <div class="panel-heading"><fmt:message key="title.common.querycondition" /></div>
            <div class="panel-body">
                <form id="formSearch" class="form-horizontal">
                    <div class="form-group">
                    <label class="control-label col-xs-1"><fmt:message key="menu.depart.departscope" />：</label>
                        <div class="col-xs-3">
                            <label class="control-label" data-bind="text:queryCondition.queryDepartTreeName"></label>
                        </div>
                        <label class="control-label col-xs-1"><fmt:message key="menu.room.roomName" />：</label>
                        <div class="col-xs-3">
                            <input type="text" class="form-control" data-bind="value:queryCondition.queryRoomName">
                        </div>
                        <div class="col-xs-3" style="text-align:right;">
                            <button type="button"data-bind="click:clearClick" class="btn"><fmt:message key="menu.common.clearbtn" /></button>
                            <button type="button"data-bind="click:queryClick" class="btn btn-primary"><fmt:message key="menu.common.querybtn"/></button>
                        </div>
                         <div class="col-xs-1">
                         <input type="hidden" id="secretKey" value="${sessionScope.secretKey}"/>
                        </div>
                    </div>
                </form>
            </div>
        </div>
        <div id="toolbar" class="btn-group">
            <button data-bind="click:addClick" type="button" class="btn btn-success">
                <span class="glyphicon glyphicon-plus" aria-hidden="true"></span><fmt:message key="menu.common.creatbtn" />
            </button>
            <button data-bind="click:editClick" type="button" class="btn btn-warning">
                <span class="glyphicon glyphicon-pencil" aria-hidden="true"></span><fmt:message key="menu.common.updatebtn" />
            </button>
            <button data-bind="click:deleteClick" type="button" class="btn btn-danger">
                <span class="glyphicon glyphicon-remove" aria-hidden="true"></span><fmt:message key="menu.common.deletebtn" />
            </button>
            <button data-bind="click:definedMethod.noticeClick" type="button" class="btn btn-info">
                <span class="glyphicon glyphicon-envelope" aria-hidden="true"></span><fmt:message key="menu.room.meetingnoticebtn" />
            </button>
            <button data-bind="click:definedMethod.queryNoticeClick" type="button" class="btn btn-info">
                <span class="glyphicon glyphicon-search" aria-hidden="true"></span><fmt:message key="menu.room.noticeviewbtn" />
            </button>
            <button data-bind="click:definedMethod.authorizedUserClick" type="button" class="btn btn-info">
                <span class="glyphicon glyphicon-user" aria-hidden="true"></span><fmt:message key="menu.room.grantuserbtn" />
            </button>
        </div>
        
        <table id ="orgtable" data-bind="bootstrapTable:bootstrapTable">
            <thead>
                <tr>
                    <th data-checkbox="true"></th>
                    <th data-field="roomID"><fmt:message key="menu.room.roomID" /></th>
                    <th data-field="roomName"><fmt:message key="menu.room.roomName" /></th>
                    <th data-field="maxUserCount"><fmt:message key="menu.room.maxUserCount" /></th>
                    <th data-field="verifyMode"><fmt:message key="menu.room.verifyMode" /></th>
                    <th data-field="curUserCount"><fmt:message key="menu.room.curUserCount" /></th>
                </tr>
            </thead>
        </table>
    </div>
</div>
</body>
</html>