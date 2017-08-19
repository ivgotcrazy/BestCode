var definedMethod={
	//是否启用参数加密传输
	isEncryption:true,
	mainKey: "roomID",
	encryptionKeyCode:$("#secretKey").val(),
	defineName:function () {
		return "";
	},
	//自定义清空方法
	defineClear:function () {
		
		cancelSelectedCss();
		initTree();
		ko.queryCondition.queryDepartID(departScope[0][0]);
		ko.queryCondition.queryDepartTreeName(departScope[0][1]);
	},
	
	//自定义新增方法
	defineQueryAdd:function () {
		var queryDepartID = ko.mapping.toJS(ko.queryCondition.queryDepartID);
		var addQuery = {key:DES3.encrypt($("#secretKey").val(),queryDepartID)};
			
		return addQuery;
	},
	//自定义修改方法
	defineQueryEdit:function (arrselectedData) {debugger
		var RoomID= arrselectedData.roomID;
		var editQuery = {key:DES3.encrypt($("#secretKey").val(),RoomID)};
		return editQuery;
	},
	//自定义删除前方法
	defineDelete:function (roomData) {
		//是否确认批量删除  1:删除 0:取消 -1:未出现确认框
		var delFlag = -1;
		//ajax 异常标志
		var errFlag = false;
		//批量删除会议室中是否存在未开始的录制任务
		$.ajax({
            url: window.BIZCTX_PATH+"/room/getNotStartRecordTask.do",
            async: false,
            cache : false,
            type: "post",
            contentType: 'application/json',
            dataType : 'json',
            data: JSON.stringify(roomData),
            success: function (data) {
            	if(!data.success){
            		var delRecordConfirm=confirm($.i18n.prop(data.data));
  					if (delRecordConfirm==false){
  						delFlag = 0;
  					}else{
  						delFlag = 1;
  					}
            	}else{
            		delFlag = -1;
				}
            },
			error:function(){
				errFlag = true;
			}
        });
		
		// ajax调用异常
		if(errFlag){
			var delConfirm=confirm($.i18n.prop('msg.roomdel.confirmdelroom'));
			if (delConfirm==false){
				return false;
			}else{
				return true;
			}
		}
		//当有录制任务时候
		// 确认删除
		if(delFlag === 1){
	   		return true;
	   	// 取消删除
		}else if (delFlag === 0){
			return false;
		}
		//当没有录制任务时候，确认是否删除
		var delConfirm=confirm($.i18n.prop('msg.roomdel.confirmdelroom'));
		if (delConfirm==false){
			return false;
		}else{
			return true;
		}
	},
	//树形数据点击查询
	queryList:function (departTreeID,departTreeName) {
		
		ko.queryCondition.queryDepartID(departTreeID);
		ko.queryCondition.queryDepartTreeName(departTreeName);
	  ko.bootstrapTable.refresh();	
	},
	//会议通知
	noticeClick:function () {
		var dialog = $('<div class="modal" id="myModal" tabindex="-1" role="dialog" aria-labelledby="myModalLabel"></div>');
		
    	var arrselectedData = ko.bootstrapTable.getSelections();
        if (arrselectedData.length <= 0 || arrselectedData.length > 1) {
        	alert($.i18n.prop("msg.common.onlyoneline"));
            return;
        }
        var RoomID= arrselectedData[0].roomID;
		var noticeQuery = {roomID:RoomID};
        //设置点击空白不关闭弹出窗口
    	dialog.modal({backdrop: 'static', keyboard: false});
    	dialog.load(window.BIZCTX_PATH+"/room/toNoticeRoomUser.do", noticeQuery, function () { });
    	
    	$("body").append(dialog);

    	dialog.modal().on('hidden.bs.modal', function () {
    	    //关闭弹出框的时候清除绑定(这个清空包括清空绑定和清空注册事件)
    	    ko.cleanNode(document.getElementById("formEdit"));
    	    dialog.remove();
    	    ko.bootstrapTable.refresh();
    	});
        
	},
	//通知查看
	queryNoticeClick:function () {
		var dialog = $('<div class="modal" id="myModal" tabindex="-1" role="dialog" aria-labelledby="myModalLabel"></div>');
		
    	var arrselectedData = ko.bootstrapTable.getSelections();
        if (arrselectedData.length <= 0 || arrselectedData.length > 1) {
        	alert($.i18n.prop("msg.common.onlyoneline"));
            return;
        }
        var RoomID= arrselectedData[0].roomID;
		var noticeQuery = {roomID:RoomID};
        //设置点击空白不关闭弹出窗口
    	dialog.modal({backdrop: 'static', keyboard: false});
    	dialog.load(window.BIZCTX_PATH+"/room/toNoticeList.do", noticeQuery, function () { });
    	
    	$("body").append(dialog);

    	dialog.modal().on('hidden.bs.modal', function () {
    	    //关闭弹出框的时候清除绑定(这个清空包括清空绑定和清空注册事件)
    	    ko.cleanNode(document.getElementById("formNotice"));
    	    dialog.remove();
    	    ko.bootstrapTable.refresh();
    	});
        
	},
	//会议室授权用户
	authorizedUserClick:function () {
		var dialog = $('<div class="modal" id="myModal" tabindex="-1" role="dialog" aria-labelledby="myModalLabel"></div>');
		
    	var arrselectedData = ko.bootstrapTable.getSelections();
        if (arrselectedData.length <= 0 || arrselectedData.length > 1) {
        	alert($.i18n.prop("msg.common.onlyoneline"));
            return;
        }
        var RoomID= arrselectedData[0].roomID;
		var noticeQuery = {key:DES3.encrypt(ko.encryptionKeyCode,RoomID)};
		
        //设置点击空白不关闭弹出窗口
    	dialog.modal({backdrop: 'static', keyboard: false});
    	dialog.load(window.BIZCTX_PATH+"/room/toAuthorizationRoomUser.do", noticeQuery, function () { });
    	
    	$("body").append(dialog);

    	dialog.modal().on('hidden.bs.modal', function () {
    	    //关闭弹出框的时候清除绑定(这个清空包括清空绑定和清空注册事件)
    	    ko.cleanNode(document.getElementById("formEdit"));
    	    dialog.remove();
    	    ko.bootstrapTable.refresh();
    	});
        
	}
}

$(function(){
		//判断是否有根节点组织
		if(rootDepartFlag == 0){
			//添加根节点弹出
			addRootDepart();
		}
		//设置查询路径
    var tableParams = {url:window.BIZCTX_PATH+"/room/roomList.do"};
    //设置删除修改添加路径
    var urls = {Delete:window.BIZCTX_PATH+"/room/deleteRoom.do",
    			edit:window.BIZCTX_PATH+"/room/toEditRoom.do",
    			add:window.BIZCTX_PATH+"/room/toAddRoom.do"};
    //设置查询对象
    var queryCondition = {queryDepartID:departScope[0][0],queryDepartTreeName:departScope[0][1],queryRoomName:""};
     //设置table参数
    var params = {sortName:"RoomID",sortOrder:"asc"};
    //设置整个查询页面model
    var modelData = {tableParams:tableParams,urls:urls,queryCondition:queryCondition,params:params};
  
    //设置table数据加载成功后的事件
    modelData.tableParams.onPreBody=function(data){
    	//循环替换级别为中文
		for(var i =0;i<data.length;i++){
			if(data[i].verifyMode== roomVerifyModeEnum[2][0]){
				data[i].verifyMode = roomVerifyModeEnum[2][1];
			}else if(data[i].verifyMode == roomVerifyModeEnum[1][0]){
				data[i].verifyMode = roomVerifyModeEnum[1][1];
			}else if(data[i].verifyMode == roomVerifyModeEnum[0][0]){
				data[i].verifyMode = roomVerifyModeEnum[0][1];
			}
		}
        
    };
    
    definedMethod.encryptionKeyCode = $("#secretKey").val();
    //绑定页面model到ko
    ko.bindingViewModel(modelData, document.getElementById("div_index"),definedMethod);
});

}