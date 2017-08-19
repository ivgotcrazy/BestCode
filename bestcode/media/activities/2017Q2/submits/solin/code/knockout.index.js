/*针对于查询页面的通用js*/
(function($) {
	ko.bindingViewModel = function(data, bindElement, definedMethod) {
		var self = this;
		//是否开启加密，开启参数加密后需要传入keyID，encryptionKeyCode 默认不开启
		this.isEncryption = false;
		if (definedMethod.isEncryption != undefined) {
			this.isEncryption = definedMethod.isEncryption;
		}
		//如果启动加密
		if (self.isEncryption) {
			this.mainKey = definedMethod.mainKey;
			this.encryptionKeyCode = definedMethod.encryptionKeyCode;
		}
		//查询参数监控已经监控绑定
		this.queryCondition = ko.mapping.fromJS(data.queryCondition);
		//自定义下拉数据监控绑定
		this.selectData = ko.mapping.fromJS(data.selectData);
		//默认查询参数
		this.defaultQueryParams = {
			queryParams : function(param) {
				var params = {};
				var queryEncryptionCondition = ko.mapping
						.toJS(self.queryCondition);
				//查询参数加密
				if (self.isEncryption) {
					for ( var name in queryEncryptionCondition) {
						queryEncryptionCondition[name] = DES3.encrypt(
								self.encryptionKeyCode,
								queryEncryptionCondition[name]);
					}
				}
				//加密自定义参数
				params.dynamicFields = queryEncryptionCondition;
				//查询参数加密
				if (self.isEncryption) {
					params.pageNumber = DES3.encrypt(self.encryptionKeyCode,
							param.pageNumber);
					params.pageSize = DES3.encrypt(self.encryptionKeyCode,
							param.pageSize);
					params.sortName = DES3.encrypt(self.encryptionKeyCode,
							data.params.sortName);
					params.sortOrder = DES3.encrypt(self.encryptionKeyCode,
							data.params.sortOrder);
				} else {
					params.pageNumber = param.pageNumber;
					params.pageSize = param.pageSize;
					params.sortName = data.params.sortName;
					params.sortOrder = data.params.sortOrder;
				}
				return params;
			}
		};

		//将传入的参数与自定义参数进行合并，相同属性不同值的时候以后者为准
		var tableParams = $.extend({}, this.defaultQueryParams,
				data.tableParams || {});
		//绑定表格数据对象
		this.bootstrapTable = new ko.bootstrapTableViewModel(tableParams);

		//绑定清空清空事件
		this.clearClick = function() {
			$.each(self.queryCondition, function(key, value) {
				//只有监控属性才清空
				if (typeof (value) == "function") {
					this(''); //value('');
				}
			});
			//清除自定义下拉
			if (ko.mapping.toJS(self.selectData) != undefined) {
				$.each(self.selectData, function(key, value) {
					//只有监控属性才清空
					if (typeof (value) == "function") {
						this(''); //value('');
					}
				});
			}
			//提供扩展定义了自定义清除事件
			if (definedMethod.defineClear != undefined) {
				definedMethod.defineClear();
			}
			//表格的刷新
			self.bootstrapTable.refresh();
		};

		//监控绑定查询事件
		this.queryClick = function() {
			//是否自定义了查询事件
			if (definedMethod.defineQuery != undefined) {
				definedMethod.defineQuery();
			}
			//表格的刷新
			self.bootstrapTable.refresh();
		};

		//新增事件
		this.addClick = function() {
			//申明默认的模态框（此处因为项目中只用到这种模态框，所以未做扩展）
			var dialog = $('<div class="modal" id="myModal" tabindex="-1" role="dialog" aria-labelledby="myModalLabel"></div>');
			//自定义点击新增前事件
			if (definedMethod.defineAdd != undefined) {
				definedMethod.defineAdd();
			}

			//自定义点击新增传参事件
			if (definedMethod.defineQueryAdd != undefined) {
				//自定义传参事件
				var addQuery = definedMethod.defineQueryAdd();
				//dialog加载页面
				dialog.load(data.urls.add, addQuery, function() {
				});
			} else {
				//加载无参数请求
				dialog.load(data.urls.add, null, function() {
				});
			}
			//body标签下追加dialog
			$("body").append(dialog);
			//dialog的关闭事件
			dialog.modal().on('hidden.bs.modal', function() {
				//关闭弹出框的时候清除绑定(这个清空包括清空绑定和清空注册事件)
				ko.cleanNode(document.getElementById("formEdit"));
				dialog.remove();
				//自定义点击新增后事件
				if (definedMethod.defineAddAfter != undefined) {
					definedMethod.defineAddAfter();
				}
				//刷新table
				self.bootstrapTable.refresh();
			});
		};

		//编辑事件
		this.editClick = function() {
			//申明默认的模态框（此处因为项目中只用到这种模态框，所以未做扩展）
			var dialog = $('<div class="modal" id="myModal" tabindex="-1" role="dialog" aria-labelledby="myModalLabel"></div>');
			//自定义点击编辑前事件
			if (definedMethod.defineEdit != undefined) {
				definedMethod.defineEdit();
			}
			//获取表格数据的选中行
			var arrselectedData = self.bootstrapTable.getSelections();
			if (arrselectedData.length <= 0 || arrselectedData.length > 1) {
				alert($.i18n.prop("msg.common.onlyoneline"));
				return;
			}
			//自定义点击编辑自定义传参事件事件
			if (definedMethod.defineQueryEdit != undefined) {
				//自定义处理传的参数
				var editQuery = definedMethod.defineQueryEdit(arrselectedData[0]);
				dialog.load(data.urls.edit, editQuery, function() {
				});
			} else {
				if (self.isEncryption) {
					//加密ID
					var dinfineData = new Array(arrselectedData.length);
					dinfineData[0] = {key : DES3.encrypt(self.encryptionKeyCode,eval("arrselectedData[0]." + self.mainKey))};
					arrselectedData[0] = dinfineData[0];
				}
				
				dialog.load(data.urls.edit, arrselectedData[0], function() {
				});
			}

			$("body").append(dialog);
			dialog.modal().on('hidden.bs.modal', function() {
				//关闭弹出框的时候清除绑定(这个清空包括清空绑定和清空注册事件)
				ko.cleanNode(document.getElementById("formEdit"));
				dialog.remove();
				//自定义点击修改后事件
				if (definedMethod.defineEditAfter != undefined) {
					definedMethod.defineEditAfter();
				}
				self.bootstrapTable.refresh();
			});
		};

		//删除事件
		this.deleteClick = function() {
			//获取表格数据的选中行
			var arrselectedData = self.bootstrapTable.getSelections();
			if (!arrselectedData || arrselectedData.length <= 0) {
				alert($.i18n.prop("msg.common.leastoneline"));
				return;
			}
			//自定义点击删除前事件
			if (definedMethod.defineDelete != undefined) {
				//自定义删除事件
				var flag = definedMethod.defineDelete(arrselectedData);
				if (flag == false) {
					return;
				}
			}
			//如果启用了参数加密删除
			if (self.isEncryption) {
				var dinfineData = new Array(arrselectedData.length);
				for (var i = 0; i < arrselectedData.length; i++) {
					dinfineData[i] = {
						key : DES3.encrypt(self.encryptionKeyCode,eval("arrselectedData[i]." + self.mainKey))
					};
				}
				arrselectedData = dinfineData;
			}

			$.ajax({
				url : data.urls.Delete,
				async : false,
				cache : false,
				type : "post",
				contentType : 'application/json',
				dataType : 'json',
				data : JSON.stringify(arrselectedData),
				success : function(data) {
					alert($.i18n.prop(data.data));
					self.bootstrapTable.refresh();
					if (definedMethod.defineDeleteAfter != undefined) {
						definedMethod.defineDeleteAfter();
					}
				},
                error: function (data) {
                	alert($.i18n.prop("msg.common.error"));
                }
			});
		};
		//扩展自定义方法
		this.definedMethod = definedMethod;
		ko.applyBindings(self, bindElement);
	};
})(jQuery);