import sys
sys.path.append("..")

from django.db import models
from users.models import ActivityUser


class CommentType(models.Model):
	comment_type_id = models.AutoField(primary_key=True)
	comment_type_name = models.CharField(max_length=64)

	def __str__(self):
		return self.comment_type_name

class Comment(models.Model):
	comment_id = models.AutoField(primary_key=True)
	comment_author = models.ForeignKey(ActivityUser, on_delete=models.CASCADE)
	comment_type = models.ForeignKey(CommentType, on_delete=models.CASCADE)
	#object_id = models.IntegerField()
	object_id = models.CharField(max_length=128)
	comment_time = models.DateTimeField("Comment Time")
	comment_text = models.TextField(max_length=4096)
	agree_count = models.IntegerField()
	disagree_count = models.IntegerField()
	reply_of_comment_id = models.IntegerField()

	def __str__(self):
		return "comment"
