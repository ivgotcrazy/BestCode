from django.contrib import admin

from .models import Submit, SubmitFile, SubmitFileType, ActivityReviewer, ReviewPlan

# Register your models here.
admin.site.register(Submit)
admin.site.register(SubmitFile)
admin.site.register(SubmitFileType)
admin.site.register(ActivityReviewer)
admin.site.register(ReviewPlan)
