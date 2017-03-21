from django.contrib import admin

from .models import Comment, CommentType

# Register your models here.
admin.site.register(Comment)
admin.site.register(CommentType)

