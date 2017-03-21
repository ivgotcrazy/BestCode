from django.db import models

# Create your models here.

class CommentType(models.Model):
	comment_type_id = models.AutoField(primary_key=True)
	comment_type_name = models.CharField(max_length=64)

class Comment(models.Model):
	comment_id = models.AutoField(primary_key=True)
	comment_type_id = models.ForeignKey(CommentType, on_delete=models.CASCADE)
	object_id = models.IntegerField()
	comment_time = models.DateTimeField()
	comment_text = models.TextField(max_length=4096)
	agree_count = models.IntegerField()
	reply_of_comment_id = models.IntegerField()
