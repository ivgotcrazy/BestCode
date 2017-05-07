import os
from django.shortcuts import render
from submit import models as submit_models
from polls import models as polls_models
from users import models as users_models
from . import models
from django.db import connection

def activity(request, activity_id):
	submits = submit_models.Submit.objects.filter(activity_id=activity_id)
	plans = models.ActivityPlan.objects.filter(activity_id=activity_id)
	submit_review_plans = submit_models.SubmitReviewPlan.objects.filter(submit__activity_id=activity_id)
	context = {
		'nav_items': [{'path': 'activities', 'text': '活动'}],
		'login': request.user.is_authenticated,
		'submits': submits,
		'plans': plans,
		'reviewers': __getActivityReviewers(activity_id, submits),
		'submit_review_plans': submit_review_plans
	}
	return render(request, 'activities/activity.html', context)	

# It's ugly, what I really want is a anonymous class
class Reviewer:
	pass

def __getActivityReviewers(activity_id, submits):
	activity_reviewers = []
	for submit in submits:
		activity_reviewer = Reviewer()
		activity_reviewer.name = submit.submit_reviewer.user.username
		activity_reviewer.department = submit.submit_reviewer.primary_department.department_name
		activity_reviewer.photo = submit.submit_reviewer.photo
		languages = []
		for language in submit.submit_reviewer.programming_languages.all():
			languages.append(language.language_name)

		activity_reviewer.languages = ""
		activity_reviewer.languages = ",".join(languages)
		activity_reviewers.append(activity_reviewer)

	return activity_reviewers
