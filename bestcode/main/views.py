from django.shortcuts import render

# Create your views here.
def index(request):
	context = { 
		'nav_items': [{'path': 'main', 'text': '首页'}],
		'registered': False,
		'activities': [
			{"title": "2016年Q4最佳代码", 
			"desc": "经过评委合议，2016年Q4最佳代码评选结果新鲜出炉，最佳代码获得者为Moon（刘轶材）和Nero（刘雍琪），恭喜两位！同时，也感谢其他参选者的积极参与和各部门对本活动的大力支持！", 
			"img_src": "main/images/baqizhao.jpg",
			"start_date": "2016年12月23日",
			"browse_times": "843"}, 
			{"title": "2016年Q4最佳代码", 
			"desc": "经过评委合议，2016年Q4最佳代码评选结果新鲜出炉，最佳代码获得者为Moon（刘轶材）和Nero（刘雍琪），恭喜两位！同时，也感谢其他参选者的积极参与和各部门对本活动的大力支持！", 
			"img_src": "main/images/baqizhao.jpg",
			"start_date": "2016年12月23日",
			"browse_times": "843"}, 
			{"title": "2016年Q4最佳代码", 
			"desc": "经过评委合议，2016年Q4最佳代码评选结果新鲜出炉，最佳代码获得者为Moon（刘轶材）和Nero（刘雍琪），恭喜两位！同时，也感谢其他参选者的积极参与和各部门对本活动的大力支持！", 
			"img_src": "main/images/baqizhao.jpg",
			"start_date": "2016年12月23日",
			"browse_times": "843"}, 
			{"title": "2016年Q4最佳代码", 
			"desc": "经过评委合议，2016年Q4最佳代码评选结果新鲜出炉，最佳代码获得者为Moon（刘轶材）和Nero（刘雍琪），恭喜两位！同时，也感谢其他参选者的积极参与和各部门对本活动的大力支持！", 
			"img_src": "main/images/baqizhao.jpg",
			"start_date": "2016年12月23日",
			"browse_times": "843"}, 
		],
 	}
	return render(request, 'main/index.html', context)
