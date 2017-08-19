var definedEditMethod={
		
	//数据保存成功后
	returnSuccess:function (data) {
		alert($.i18n.prop(data.data));
        $("#myModal").modal("hide");
	},
	//数据保存失败后
	returnFail:function (data) {
		$('#formEdit').bootstrapValidator('disableSubmitButtons', false);
	},
	//提交前校验
	submitCheck:function (model) {
		//验证会议室名是否为空
		if(model.roomName.replace(/(^\s+|\s+$)/g,"") == ""){
			alert($.i18n.prop('msg.room.roomnameempty'));
			$("[name=txt_roomName]").focus();
			return false;
		}
		//验证会议室名规则与长度是否合法
		if(!checkRoomName(model.roomName)){
			alert($.i18n.prop('msg.room.roomnameformaterror'));
			$("[name=txt_roomName]").focus();
			return false;
		}
		//验证会议室最大人数是否为空
		if(model.maxUserCount == ""){
			alert($.i18n.prop('msg.room.maxusercountempty'));
			$("[name=txt_maxUserCount]").focus();
			return false;
		}
		//验证最大人数规则与大小是否合法
		if(!checkMaxUserCount(model.maxUserCount)){
			alert($.i18n.prop('msg.room.maxusercountformaterror'));
			$("[name=txt_maxUserCount]").focus();
			return false;
		}
		//如果启用主席密码不为空验证主席密码
		if(model.enableChairPwd.replace(/(^\s+|\s+$)/g,"") == "1"){
			//验证会议室主席密码是否为空
			if(model.chairPassword.replace(/(^\s+|\s+$)/g,"") == ""){
				alert($.i18n.prop('msg.room.chairpasswordempty'));
				$("[name=txt_chairPassword]").focus();
				return false;
			}
			//验证主席密码规则与大小是否合法
			if(!checkChairPassword(model.chairPassword)){
				alert($.i18n.prop('msg.room.chairpasswordformaterror'));
				$("[name=txt_chairPassword]").focus();
				return false;
			}
		}
		//如果登录校验模型为会议室密码登录校验会议室密码
		if(model.verifymode == "2"){
			//验证会议室密码是否为空
			if(model.password.replace(/(^\s+|\s+$)/g,"") == ""){
				alert($.i18n.prop('msg.room.passwordempty'));
				$("[name=txt_password]").focus();
				return false;
			}
			//验证密码规则与大小是否合法
			if(!checkPassword(model.password)){
				alert($.i18n.prop('msg.room.passwordformaterror'));
				$("[name=txt_password]").focus();
				return false;
			}
		}
		//校验会议室描述长度是否符合规则
		if(!checkLength(model.roomDesc,roomDescLengthRule[0],roomDescLengthRule[1])){
			alert($.i18n.prop('msg.room.roomdesclengtherror'));
			$("[name=txt_roomDesc]").focus();
			$('#formEdit').bootstrapValidator('disableSubmitButtons', false);
			return false;
		}
		
		//如果登录校验模型为会议室密码登录校验会议室密码
		if(model.useDefaultFlag == "1"){
			//验证会议室码流是否为空
			if(model.defaultVideoBitrate == ""){
				alert($.i18n.prop('msg.room.defaultvideobitrateempty'));
				$("[name=txt_defaultVideoBitrate]").focus();
				return false;
			}
			//验证码流规则与大小是否合法
			if(!checkDefaultVideoBitrate(model.defaultVideoBitrate)){
				alert($.i18n.prop('msg.room.defaultvideobitrateformaterror'));
				$("[name=txt_defaultVideoBitrate]").focus();
				return false;
			}
		}
		
		//如果会议室类型为周例会议室 并且循环模式为每月几号
		if(model.roomType == "4" && model.cycleFlag == "3"){
			//验证会议室每月几号是否为空
			if(model.dateEveyMonth == ""){
				alert($.i18n.prop('msg.room.dateeveymonthempty'));
				$("[name=txt_dateEveyMonth]").focus();
				return false;
			}
			//验证码流规则与大小是否合法
			if(!checkDateEveyMonth(model.dateEveyMonth)){
				alert($.i18n.prop('msg.room.dateeveymonthlengtherror')+" "+$.i18n.prop('msg.room.dateeveymontherror'));
				$("[name=txt_dateEveyMonth]").focus();
				return false;
			}
		}
		
		//如果会议室类型为周例会议室 
		if(model.roomType == "4"){
			//验证会议室预约开始时间是否为空
			if(model.weekStartTime.replace(/(^\s+|\s+$)/g,"") == ""){
				alert($.i18n.prop('msg.room.weekstarttimeempty'));
				$("[name=txt_weekStartTime]").focus();
				return false;
			}
			//验证会议室预约结束时间是否为空
			if(model.weekEndTime.replace(/(^\s+|\s+$)/g,"") == ""){
				alert($.i18n.prop('msg.room.weekendtimeempty'));
				$("[name=txt_weekEndTime]").focus();
				return false;
			}
			//验证结束时间是否大于开始时间
			if(new Date("1970/01/01"+model.weekStartTime) > new Date("1970/01/01"+model.weekEndTime)){
				alert($.i18n.prop('msg.room.weektimeunreasonable'));
				$("[name=txt_weekEndTime]").focus();
				return false;
			}
		}
		
		//如果会议室类型为预约会议室 
		if(model.roomType == "2"){
			//验证会议室预约开始时间是否为空
			if(model.hopeStartTime.replace(/(^\s+|\s+$)/g,"") == ""){
				alert($.i18n.prop('msg.room.hopestarttimeempty'));
				$("[name=txt_hopeStartTime]").focus();
				return false;
			}
			//验证会议室预约结束时间是否为空
			if(model.hopeEndTime.replace(/(^\s+|\s+$)/g,"") == ""){
				alert($.i18n.prop('msg.room.hopeendtimeempty'));
				$("[name=txt_hopeEndTime]").focus();
				return false;
			}
			//验证结束时间是否大于开始时间
			if(new Date(model.hopeStartTime) > new Date(model.hopeEndTime)){
				alert($.i18n.prop('msg.room.hopetimeunreasonable'));
				$("[name=txt_hopeEndTime]").focus();
				return false;
			}
		}
		
		
		return true;
	}
}

//自定义数组转对象arrayToObject(defaultVideoWindEnum)
function defineArrayToObjectArray(arrayEnum,definePremise){
	var objArray = new Array();
	for(var i = 0;i < arrayEnum.length;i++){
		var obj = {};
		obj.id = arrayEnum[i][0];
		obj.name = arrayEnum[i][1];
		if(arrayEnum[i][1].substring(0,arrayEnum[i][1].indexOf("*")) <= definePremise){
			objArray[i] = obj;
		}
	}
	
	return objArray;
}

$(function() {
	//组织级别下拉model
	var verifyModeModel = function() {
		this.items = ko.observableArray( arrayToObjectArray(roomVerifyModeEnum) );
		this.selected = ko.observable();
	};
	var verifyModeData = new verifyModeModel();

	//会议室类型下拉model
	var roomTypeModel = function() {
		this.items = ko.observableArray(arrayToObjectArray(roomTypeEnum));
		this.selected = ko.observable();
	};
	var roomTypeModelData = new roomTypeModel();
	
	//会议室状态下拉
	var roomStatusModel = function() {
		this.items = ko.observableArray(arrayToObjectArray(roomStatusEnum));
		this.selected = ko.observable();
	};
	var roomStatusData = new roomStatusModel();
	
	//会议室默认视频解码下拉model
	var defaultVideoCodecModel = function() {
		this.items = ko.observableArray(arrayToObjectArray(defaultVideoCodecEnum));
		this.selected = ko.observable();
	};
	var defaultVideoCodecData = new defaultVideoCodecModel();
	//会议室视频分辨率下拉model
	var defaultVideoWindModel = function() {
		this.items = ko.observableArray(defineArrayToObjectArray(defaultVideoWindEnum,maxVideoStr));
		this.selected = ko.observable();
	};
	var defaultVideoWindData = new defaultVideoWindModel();
	
	//会议室开会的角色下拉model
	var meetingRoleModel = function() {
		this.items = ko.observableArray(arrayToObjectArray(meetingRoleEnum));
		this.selected = ko.observable();
	};
	var meetingRoleData = new meetingRoleModel();
	
	//上级组织model
	var parentModel = function() {
		this.items = ko.observableArray(subDepartList);
		this.selected = ko.observable();
	};
	var parentData = new parentModel();

	//下拉对象
	var selectData = {
		verifyModeData : verifyModeData,
		roomTypeModelData:roomTypeModelData,
		roomStatusData:roomStatusData,
		defaultVideoCodecData:defaultVideoCodecData,
		defaultVideoWindData:defaultVideoWindData,
		meetingRoleData:meetingRoleData,
		parent : parentData
	};

	//新建组织model
	var editModel = room;
	var urls = {
		submit : room.roomID == 0 ? window.BIZCTX_PATH
				+ "/room/addRoom.do" : window.BIZCTX_PATH
				+ "/room/editRoom.do"
	};
	var editData = {
		editModel : editModel,
		urls : urls,
		selectData : selectData
	};

	var addDepartValidator = {
		txt_roomName : {
			validators : {
				notEmpty : {
					message : $.i18n.prop('msg.room.roomnameempty')
				},
				stringLength : {
					min : roomNameLengthRule[0],
					max : roomNameLengthRule[1],
					message : $.i18n.prop('msg.room.roomnamelengtherror')
				},
				regexp : {
					regexp : roomNameRule,
					message : $.i18n.prop('msg.room.roomnameformaterror')
				}
			}
		},
		txt_maxUserCount : {
			validators : {
				notEmpty : {
					message : $.i18n.prop('msg.room.maxusercountempty')
				},
				between : {
					min : maxUserCountLengthRule[0],
					max : maxUserCountLengthRule[1],
					message : $.i18n.prop('msg.room.maxusercountlengtherror')
				},
				regexp : {
					regexp : maxUserCountRule,
					message : $.i18n.prop('msg.room.maxusercountformaterror')
				}
			}
		},
		txt_chairPassword : {
			validators : {
				notEmpty : {
					message : $.i18n.prop('msg.room.chairpasswordempty')
				},
				stringLength : {
					min : chairPasswordLengthRule[0],
					max : chairPasswordLengthRule[1],
					message : $.i18n.prop('msg.room.chairpasswordlengtherror')
				},
				regexp : {
					regexp : chairPasswordRule,
					message : $.i18n.prop('msg.room.chairpasswordformaterror')
				}
			}
		},
		txt_password : {
			validators : {
				notEmpty : {
					message : $.i18n.prop('msg.room.passwordempty')
				},
				stringLength : {
					min : passwordLengthRule[0],
					max : passwordLengthRule[1],
					message : $.i18n.prop('msg.room.passwordlengtherror')
				},
				regexp : {
					regexp : passwordRule,
					message : $.i18n.prop('msg.room.passwordformaterror')
				}
			}
		},
		txt_roomDesc : {
			validators : {
				stringLength : {
					min : roomDescLengthRule[0],
					max : roomDescLengthRule[1],
					message : $.i18n.prop('msg.room.roomdesclengtherror')
				}
			}
		},
		txt_defaultVideoBitrate : {
			validators : {
				notEmpty : {
					message : $.i18n.prop('msg.room.defaultvideobitrateempty')
				},
				between : {
					min : defaultVideoBitrateLengthRule[0],
					max : defaultVideoBitrateLengthRule[1],
					message : $.i18n.prop('msg.room.defaultvideobitratelengtherror')
				},
				regexp : {
					regexp : defaultVideoBitrateRule,
					message : $.i18n.prop('msg.room.defaultvideobitrateformaterror')
				}
			}
		},
		txt_dateEveyMonth : {
			validators : {
				notEmpty : {
					message : $.i18n.prop('msg.room.dateeveymonthempty')
				},
				between : {
					min : dateEveyMonthLengthRule[0],
					max : dateEveyMonthLengthRule[1],
					message : $.i18n.prop('msg.room.dateeveymonthlengtherror')
				},
				regexp : {
					regexp : dateEveyMonthRule,
					message : $.i18n.prop('msg.room.dateeveymontherror')
				}
			}
		}
	};
	ko.bindingEditViewModel(editData, addDepartValidator,definedEditMethod);
	
	changeChairPasswordStatus();
	changePasswordStatus();
});



//手动赋值给预约开始时间
function pickedhopeStartTime(dp){
	ko.editModel.hopeStartTime(dp.cal.getNewDateStr());
}
//手动赋值给预约结束时间
function pickedhopeEndTime(dp){
	ko.editModel.hopeEndTime(dp.cal.getNewDateStr());
}

function pickedweekStartTime(dp){
	ko.editModel.weekStartTime(dp.cal.getNewDateStr());
}

function pickedweekEndTime(dp){
	ko.editModel.weekEndTime(dp.cal.getNewDateStr());
}
//改变是否启动会议室密码
function changeChairPasswordStatus(){
	debugger
	var flag = ko.mapping.toJS(ko.editModel.enableChairPwd);
	

		if (flag == 1) {
		$("#pwd_chairPassword").attr("disabled", false);
	} else {
		$("#pwd_chairPassword").attr("disabled", true);
		$('#formEdit').data('bootstrapValidator').updateStatus(
				'txt_chairPassword', 'NOT_VALIDATED', null).validateField(
				'txt_chairPassword');
		ko.editModel.chairPassword("");
	}
	
};
//改变是否启动会议室密码
function changePasswordStatus(){
	
	var flag = ko.mapping.toJS(ko.editModel.verifyMode);
	
	if(flag == 2){
		$("#pwd_password").attr("disabled",false);
	}else{
		$("#pwd_password").attr("disabled",true);
		$('#formEdit').data('bootstrapValidator').updateStatus(
				'txt_password', 'NOT_VALIDATED', null).validateField(
				'txt_password');
		ko.editModel.password("");
	}
	
};
//改变默认视频默认设置参数是否
function changeDefaultSetStatus(){
	
	var flag = ko.mapping.toJS(ko.editModel.useDefaultFlag);
	if(flag != 1){
		$('#formEdit').data('bootstrapValidator').updateStatus(
				'txt_defaultVideoBitrate', 'NOT_VALIDATED', null).validateField(
				'txt_defaultVideoBitrate');
		ko.editModel.defaultVideoBitrate(512);
	}
	
};
//改变周例会议室固定定期
function changeCycleFlagStatus(){
	
	var flag = ko.mapping.toJS(ko.editModel.cycleFlag);
	if(flag != 3){
		$('#formEdit').data('bootstrapValidator').updateStatus(
				'txt_dateEveyMonth', 'NOT_VALIDATED', null).validateField(
				'txt_dateEveyMonth');
		ko.editModel.dateEveyMonth("");
	}
	
};