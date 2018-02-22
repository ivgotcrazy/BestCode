

//课表左右移动
var coursePlayf = [];
for(var i=0; i<$('.courseListUl').length; i++){
    coursePlayf[i] = new CoursePlay('.courseListUl',i);
}


//课程展示（课表）
function CoursePlay(course,n){
    //下面是设置coursePlay的属性
    this.times = 0;//初始化计数器
    this.course = $(course).eq(n);//选到ul
    this.courseInfo = this.course.find(".courseInfo");//选到ul的li
    this.hasClass = this.course.closest(".courseList").siblings('.timeInfo').find(".has-class");//取到刻度上有课程的块
    this.multiple = Math.ceil(this.courseInfo.length / 4);//对4取余向上取舍（假如有10个li 10/4=3）
    this.leftMove = this.course.parent().siblings(".leftMove");//选取左边按钮
    this.rightMove = this.course.parent().siblings(".rightMove");//选取右边按钮
    this.titleActive();
    //初始化左右按钮的状态（可点/不可点）
    this.checkButtonStatus();
    var _this = this;
    //点击左移
    this.leftMove.click(function(){
        _this.moveToLeft();
    });
    //点击右移
    this.rightMove.click(function () {
        _this.moveToRight();
    });
    //悬浮时间刻度上的块进行左右移
    this.hasClass.hover(function(){//移入
        for(var i=0; i < _this.courseInfo.length; i++){//遍历刻度上有课程的块
            if($(this).attr('title') == _this.courseInfo.find(".classTime").find("b").eq(i).text()){
                _this.courseInfo.eq(i).css("box-shadow", "0px 0px 5px #000 inset");
                var timesNext = Math.floor(i/4);//假如悬浮的是第5块，5/4=1
                if(timesNext > _this.times){//1>0
                    _this.times = timesNext - 1;
                    _this.moveToLeft();
                }else if(timesNext < _this.times){
                    _this.times = timesNext + 1;
                    _this.moveToRight();
                }
                return;
            }
        }
    },function(){//移出
        for(var i=0; i < _this.courseInfo.length; i++){
            if($(this).attr('title') == _this.courseInfo.find(".classTime").find("b").eq(i).text()){
                _this.courseInfo.eq(i).css("box-shadow", "none");
            }
        }
    });
}

//左移
CoursePlay.prototype.moveToLeft = function(){
    //假设li的长度为10，则 this.multiple=3
    var a = ++this.times;//times初始是0，左点击一次，times+1，则a=1
    var courseWidth = this.course.width(this.course.find(".courseInfo").length * 283.7);
    if (a < this.multiple) {//1<3
        this.course.stop().animate({"left": -283.7 * 4 * a + "px"}, 1000);//向左平移4个li
    } else {
        this.times = this.multiple-1;//将times设置为3-1=2
    }
    this.titleActive();
    this.checkButtonStatus();
};

//右移
CoursePlay.prototype.moveToRight = function(){
    var b = --this.times;
    var courseWidth = this.course.width(this.course.find(".courseInfo").length * 283.7);
    if (b >= 0) {
        this.course.stop().animate({"left": -283.7 * 4 * b + "px"}, 1000);
    } else {
        this.times = 0;
    }
    this.titleActive();
    this.checkButtonStatus();
};

//高亮时间刻度
CoursePlay.prototype.titleActive = function(){
    this.hasClass.removeClass('selected');
    var pageStart = this.times*4;
    var pageEnd = (this.times+1)*4;
    for(var i=pageStart; i<pageEnd; i++){//根据times的值来选中对应的时间刻度，若times=1，选中的是第四个到第八个。
        this.hasClass.eq(i).addClass('selected');
    }
};

//左右按钮状态
CoursePlay.prototype.checkButtonStatus = function(){
    if(this.times <= 0){//右按钮的显示状态
        this.rightMove.css({"cursor":"not-allowed","color": "#eeeee"});
    }else{
        this.rightMove.css({"cursor":"pointer","color": "#d2d2d2"});
    }
    if(this.times >= this.multiple-1){//左按钮的显示状态
        this.leftMove.css({"cursor":"not-allowed","color": "#eeeeee"});
    }else{
        this.leftMove.css({"cursor":"pointer","color": "#d2d2d2"});
    }
};

