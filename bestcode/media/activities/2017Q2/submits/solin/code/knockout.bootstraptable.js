//初始化
(function($) {
	//向ko里面新增一个bootstrapTableViewModel方法
	ko.bootstrapTableViewModel = function(options) {
		var that = this;
		//bootstarp默认参数设置
		this.defaultSetting = {
			toolbar: '#toolbar',                //工具按钮用哪个容器
			queryParamsType:"",									//数据查询l类型
			pagination: true,                   //是否显示分页（*）
			sidePagination: "server",           //分页方式：client客户端分页，server服务端分页（*）
			pageNumber: 1,                      //初始化加载第一页，默认第一页
			pageSize: 15,                       //每页的记录行数（*）
			pageList: [15, 25, 50, 100],        //可供选择的每页的行数（*）
			method: 'post',											//请求方式
			search: false,                      //是否显示表格搜索，此搜索是客户端搜索，不会进服务端，所以，个人感觉意义不大
			strictSearch: true,									//数据严格搜索,true启用全匹配搜索，否则为模糊搜索
			showColumns: true,                  //是否显示所有的列
			cache:false,												//设置为 false 禁用 AJAX 数据缓存
			showRefresh: true,                  //是否显示刷新按钮
			minimumCountColumns: 2,             //最少允许的列数
			clickToSelect: true,                //是否启用点击选中行
			showToggle: true,										//数据显示，切换
			striped: true,											//各行变色
			sortStable: true,										//排序
		};

		//将传进来的参数与默认参数做一个合并，相同属性不同值的时候以后者为准
		this.params = $.extend({}, this.defaultSetting, options || {});

		//得到选中的记录
		this.getSelections = function() {
			var arrRes = that.bootstrapTable("getSelections");
			arrRes = removeExtraFields(arrRes);
			return arrRes;
		};
		
		//得到所有的选中的记录
		this.getAllSelections = function() {
			var arrRes = that.bootstrapTable("getAllSelections");
			arrRes = removeExtraFields(arrRes);
			return arrRes;
		};
		
		//得到回调数据
		this.getAllData = function() {
			var arrRes = that.bootstrapTable("getData");
			
			return arrRes;
		};
		
		//刷新
		this.refresh = function() {
			that.bootstrapTable("refresh");
		};
		//prepend插入数据到表格在现有数据之前。
		this.prepend = function(data) {
			that.bootstrapTable("prepend",data);
		};
		//load加载数据到表格中，旧数据会被替换。
		this.load = function(data) {
			that.bootstrapTable("load",data);
		};
		
		//removeAll删除表格所有数据。
		this.removeAll = function() {
			that.bootstrapTable("removeAll");
		};
		
		//remove删除表格指定数据。
		this.remove = function(data) {
			that.bootstrapTable("remove",data);
		};
	};

	//剔除选中列的值
	function removeExtraFields(arrRes){
		for(var i = 0;i < arrRes.length;i++){
			delete(arrRes[i]["0"]);
		}
		return arrRes;
	}
	
//添加ko自定义绑定
/*element - 涉及此绑定的DOM元素
valueAccessor - 一个JavaScript函数，您可以调用此函数来获取此绑定所涉及的当前模型属性。调用它而不传递任何参数（即调用valueAccessor()）来获取当前的模型属性值。
allBindingsAccessor - 可用于访问绑定到此DOM元素的所有模型值的JavaScript对象
*bindingContext- 保存绑定上下文可用于此元素绑定的对象。
*/
ko.bindingHandlers.bootstrapTable = {
	init : function(element, valueAccessor, allBindingsAccessor, viewModel,bindingContext) {debugger
		//这里的oParam就是绑定的viewmodel,valueAccessor()=ko.bootstrapTableViewModel
		var oViewModel = valueAccessor();
		//根据参数执行查询方法
		var $ele = $(element).bootstrapTable(oViewModel.params);
		
		//劫持方式给oViewModel添加bootstrapTable方法
		oViewModel.bootstrapTable = function() {
			return $ele.bootstrapTable.apply($ele, arguments);
		}
	},
	update : function(element, valueAccessor, allBindingsAccessor, viewModel,bindingContext) {
	}
};

})(jQuery);