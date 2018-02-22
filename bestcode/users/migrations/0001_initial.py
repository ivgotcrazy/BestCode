# -*- coding: utf-8 -*-
# Generated by Django 1.10 on 2017-08-28 04:12
from __future__ import unicode_literals

from django.conf import settings
from django.db import migrations, models
import django.db.models.deletion


class Migration(migrations.Migration):

    initial = True

    dependencies = [
        migrations.swappable_dependency(settings.AUTH_USER_MODEL),
    ]

    operations = [
        migrations.CreateModel(
            name='ActivityUser',
            fields=[
                ('activity_user_id', models.AutoField(primary_key=True, serialize=False)),
                ('chinese_name', models.CharField(default='未指定', max_length=32)),
                ('photo', models.ImageField(upload_to='users/photos')),
            ],
        ),
        migrations.CreateModel(
            name='PrimaryDepartment',
            fields=[
                ('primary_department_id', models.AutoField(primary_key=True, serialize=False)),
                ('department_name', models.CharField(max_length=128)),
            ],
        ),
        migrations.CreateModel(
            name='ProgrammingLanguage',
            fields=[
                ('programming_language_id', models.AutoField(primary_key=True, serialize=False)),
                ('language_name', models.CharField(max_length=64)),
            ],
        ),
        migrations.CreateModel(
            name='SecondaryDepartment',
            fields=[
                ('secondary_department_id', models.AutoField(primary_key=True, serialize=False)),
                ('department_name', models.CharField(max_length=128)),
            ],
        ),
        migrations.AddField(
            model_name='activityuser',
            name='primary_department',
            field=models.ForeignKey(on_delete=django.db.models.deletion.CASCADE, to='users.PrimaryDepartment'),
        ),
        migrations.AddField(
            model_name='activityuser',
            name='programming_languages',
            field=models.ManyToManyField(to='users.ProgrammingLanguage'),
        ),
        migrations.AddField(
            model_name='activityuser',
            name='secondary_department',
            field=models.ForeignKey(on_delete=django.db.models.deletion.CASCADE, to='users.SecondaryDepartment'),
        ),
        migrations.AddField(
            model_name='activityuser',
            name='user',
            field=models.OneToOneField(on_delete=django.db.models.deletion.CASCADE, to=settings.AUTH_USER_MODEL),
        ),
    ]
