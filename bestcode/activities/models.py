from django.db import models


class ActivityState(models.Model):
	state_id = models.AutoField(primary_key=True)
	state_name = models.CharField(max_length=64)

	def __str__(self):
		return self.state_name

class Activity(models.Model):
	activity_id = models.AutoField(primary_key=True)
	name = models.CharField(max_length=64, default="最佳代码")
	desc = models.TextField(max_length=4096)
	result = models.TextField(max_length=4096)
	img_path = models.CharField(max_length=256, default="")
	state = models.ForeignKey(ActivityState, on_delete=models.CASCADE)
	start_date = models.DateTimeField('Start Time')
	browse_times = models.IntegerField('Browse Times')

	def __str__(self):
		return self.name 
	
class ActivityPlan(models.Model):
	activity_plan_id = models.AutoField(primary_key=True)
	activity = models.ForeignKey(Activity, on_delete=models.CASCADE)
	plan_item_name = models.CharField(max_length=64)
	plan_item_start = models.DateTimeField('Start time')
	plan_item_end = models.DateTimeField('End time')

	def __str__(self):
		return "activity plan of %s" % self.activity_id
