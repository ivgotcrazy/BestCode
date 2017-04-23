import sys
sys.path.append("..")

from django.db import models
from django.contrib.auth import models as auth_models
from users.models import ProgrammingLanguage
from activities.models import Activity


# Create your models here.

class Submit(models.Model):
	submit_id = models.AutoField(primary_key=True)
	user = models.ForeignKey(auth_models.User, on_delete=models.CASCADE)
	activity = models.ForeignKey(Activity, on_delete=models.CASCADE)
	programming_language = models.ForeignKey(ProgrammingLanguage, on_delete=models.CASCADE)

	def __str__(self):
		return "%s's submit for %s" % (self.user_id, self.activity_id)

class SubmitFileType(models.Model):
	submit_file_type_id = models.AutoField(primary_key=True)
	sumbmit_file_type_name = models.CharField(max_length=64)

	def __str__(self):
		return self.submit_file_type_name

class SubmitFile(models.Model):
	submit_file_id = models.AutoField(primary_key=True)
	submit_file_type = models.ForeignKey(SubmitFileType, on_delete=models.CASCADE)
	submit = models.ForeignKey(Submit, on_delete=models.CASCADE)
	file_path = models.CharField(max_length=256)
	
	def __str__(self):
		return self.file_path	

# this should be in activities app, but for cycle dependence
class SubmitReviewPlan(models.Model):
    submit_review_plan_id = models.AutoField(primary_key=True)
    submit = models.ForeignKey(Submit, on_delete=models.CASCADE)
    plan_item_start = models.DateTimeField('Start time')
    plan_item_end = models.DateTimeField('End time')

    def __str__(self):
        return "Review plan for %s" % self.submit_id

# this should be in activities app, but for cycle dependence
class SubmitReviewer(models.Model):
    submit_reviewer_id = models.AutoField(primary_key=True)
    submit = models.ForeignKey(Submit, on_delete=models.CASCADE)
    user = models.ForeignKey(auth_models.User, on_delete=models.CASCADE)

    def __str__(self):
        return "%s review for %s" % (self.user_id, self.submit_id)
