(function ($) {
    ko.bindingEditViewModel = function (data, validatorFields,definedEditMethod) {
        var that = this;
        //修改新增的对象添加监控并绑定
    	this.editModel = ko.mapping.fromJS(data.editModel);
        //如果有下拉对象则监控
        if(data.selectData != undefined){
        	this.selectData = ko.mapping.fromJS(data.selectData);
        }
        //默认新增修改配置
        this.Default = {
            message: $.i18n.prop('msg.common.checknotpassed'),
            fields: validatorFields,
            //bootstrapValidator的提交方法，验证通过才能提交
            submitHandler: function (validator, form, submitButton) {
            	//获取新增编辑的对象并解除监控
                var arrselectedData = ko.mapping.toJS(that.editModel);
                //自定义参数JS参数校验方法
                if(definedEditMethod.submitCheck != undefined){
                	var flag = definedEditMethod.submitCheck(arrselectedData);
                  	 if(flag == false){
                  		 return;
                  	 }
        		}
                
                $.ajax({
                    url:data.urls.submit,
                    type: "POST",
                    async: false,
                    cache : false,
                    contentType: 'application/json',
                    dataType : 'json',
                    data: JSON.stringify(arrselectedData),
                    success: function (data) {
                    	if(data.success){
                    		if(definedEditMethod.returnSuccess != undefined){
                    			definedEditMethod.returnSuccess(data);
                    		}else{
                    			 alert($.i18n.prop(data.data));
                                 $("#myModal").modal("hide");
                    		}
                    	}else{
                    		
                    		if(definedEditMethod.returnFail != undefined){
                    			definedEditMethod.returnFail(data);
                    		}
                    		alert($.i18n.prop(data.data));
                    	}
                       
                    },
                    error: function (data) {
                    	if(definedEditMethod.returnFail != undefined){
                    		alert("error");
                    		definedEditMethod.returnFail(data);
                		}
                    	alert($.i18n.prop(data.data));
                    }
                });
               
            }
        };
        this.definedEditMethod = definedEditMethod;
        this.params = $.extend({}, that.Default, {fields: validatorFields} || {});
        $('#formEdit').bootstrapValidator(that.params);
        
        ko.applyBindings(that, document.getElementById("formEdit"));
    };

})(jQuery);