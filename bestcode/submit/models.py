import sys
sys.path.append("..")

from django.db import models
from users.models import ProgrammingLanguage, ActivityUser
from activities.models import Activity


# Create your models here.

class Submit(models.Model):
	submit_id = models.AutoField(primary_key=True)
	activity = models.ForeignKey(Activity, on_delete=models.CASCADE)
	activity_user = models.ForeignKey(ActivityUser, on_delete=models.CASCADE, related_name='activity_user')
	submit_reviewer = models.ForeignKey(ActivityUser, on_delete=models.CASCADE, related_name='submit_reviewer')
	programming_language = models.ForeignKey(ProgrammingLanguage, on_delete=models.CASCADE)
	code_desc = models.TextField(max_length=4096)
	praise_count = models.IntegerField(default=0)
	browse_times = models.IntegerField(default=0)

	def __str__(self):
		return "%s%s的提交" % (self.activity.name, self.activity_user.user.username)

class SubmitFileType(models.Model):
	submit_file_type_id = models.AutoField(primary_key=True)
	submit_file_type_name = models.CharField(max_length=64)

	def __str__(self):
		return self.submit_file_type_name

class SubmitFile(models.Model):
	submit_file_id = models.AutoField(primary_key=True)
	submit_file_type = models.ForeignKey(SubmitFileType, on_delete=models.CASCADE)
	submit = models.ForeignKey(Submit, on_delete=models.CASCADE)
	file_path = models.CharField(max_length=256)
	file_name = models.CharField(max_length=128, default='')
	
	def __str__(self):
		return self.file_path	

# this should be in activities app, but for cycle dependence
class SubmitReviewPlan(models.Model):
    submit_review_plan_id = models.AutoField(primary_key=True)
    submit = models.ForeignKey(Submit, on_delete=models.CASCADE)
    plan_item_start = models.DateTimeField('Start time')
    plan_item_end = models.DateTimeField('End time')

    def __str__(self):
        return "Review plan for %s" % self.submit
